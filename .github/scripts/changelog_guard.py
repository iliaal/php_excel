#!/usr/bin/env python3
"""Changelog guard for Keep a Changelog style CHANGELOG.md files.

MODE=pr (pull_request): exits 1 when merging the PR into the base branch would
  - conflict on the changelog,
  - add, remove or change a line inside a released section, meaning a `## [x]` heading other than
    Unreleased that already exists on the base branch (the override label turns this into a warning),
  - add a duplicate `## [x]` heading, a second `### Name` heading in one section, or an entry that
    repeats another entry under the same heading,
  - or, together with a lower-numbered open PR that also edits the changelog, conflict on it or
    produce one of those duplicates. The later PR is the one that fails.
  Anything else inside [Unreleased], or inside a section the PR adds, is allowed: new headings,
  rewording and removals included. A PR that doesn't touch the changelog is not checked.

MODE=recheck (push to the base branch that touches the changelog): runs the base checks above for
every open PR that edits the changelog and sets a `changelog-guard/base` commit status on its head
commit, failure or success. Always exits 0, because the push itself is not what's being checked.

Env: MODE, BASE (commit or ref to merge into), BASE_BRANCH, REPO (owner/name), PR_NUMBER and LABELS
(comma-separated) in pr mode, CHANGELOG (default CHANGELOG.md), OVERRIDE_LABEL (default
changelog-override), RUN_URL (optional link for statuses), GH (gh binary, default gh).
"""
import difflib, json, os, re, subprocess, sys

CL = os.environ.get("CHANGELOG", "CHANGELOG.md")
OVERRIDE = os.environ.get("OVERRIDE_LABEL", "changelog-override")
GH = os.environ.get("GH", "gh")
H2 = re.compile(r"^## \[([^\]]+)\]")
H3 = re.compile(r"^### +(.+?)\s*$")
BULLET = re.compile(r"^[-*+]\s+(.*\S)")  # top-level entries only
LINKREF = re.compile(r"^\[[^\]]+\]:\s")  # compare links at the bottom are release bookkeeping


def run(*cmd, ok=(0,)):
    p = subprocess.run(cmd, capture_output=True, text=True)
    if p.returncode not in ok:
        sys.exit(f"{' '.join(cmd)} failed: {p.stderr.strip()}")
    return p


def show(rev):
    p = run("git", "show", f"{rev}:{CL}", ok=(0, 128))
    return p.stdout.splitlines() if p.returncode == 0 else None


def merge(a, b):
    p = run("git", "merge-tree", "--write-tree", "--name-only", "--no-messages", a, b, ok=(0, 1))
    out = p.stdout.splitlines()
    return out[0], [f for f in out[1:] if f]


def touches(base, head):
    return run("git", "diff", "--quiet", f"{base}...{head}", "--", CL, ok=(0, 1)).returncode == 1


def sections(lines):
    cur, res = None, []
    for l in lines:
        m = H2.match(l)
        cur = m.group(1) if m else cur
        res.append(cur)
    return res


def released_of(lines):
    return {s for s in sections(lines) if s and s.lower() != "unreleased"}


def norm(s):
    return re.sub(r"\s+", " ", s).strip().lower()


def dup_keys(lines, released):
    """Duplicate ## headings anywhere; duplicate ### headings and entries inside editable sections."""
    seen, dups, h3 = set(), set(), None
    for sec, l in zip(sections(lines), lines):
        if (m := H2.match(l)):
            k, h3 = ("section", m.group(1)), None
        elif sec in released:
            continue
        elif (m := H3.match(l)):
            k, h3 = ("heading", sec, m.group(1)), m.group(1)
        elif (m := BULLET.match(l)):
            k = ("entry", sec, h3, norm(m.group(1)))
        else:
            continue
        (dups if k in seen else seen).add(k)
    return dups


def describe(k):
    if k[0] == "section":
        return f"`## [{k[1]}]` appears twice"
    if k[0] == "heading":
        return f"`### {k[2]}` appears twice under [{k[1]}]; put the entry under the existing heading"
    return f"entry `{k[3][:80]}` appears twice under [{k[1]}]"


def check_base(base, head, label, allow_released):
    """Diff the base branch's changelog against the merge result. Returns (errors, warnings)."""
    tree, conflicts = merge(base, head)
    if CL in conflicts:
        return [f"{label}: {CL} conflicts with the base branch; update the branch and keep entries under [Unreleased]"], []
    old, new = show(base) or [], show(tree)
    if new is None:
        return [], []
    osec, nsec, released = sections(old), sections(new), released_of(old)
    hits = []
    for tag, i1, i2, j1, j2 in difflib.SequenceMatcher(None, old, new, autojunk=False).get_opcodes():
        if tag == "equal":
            continue
        hits += [f"changes released section [{osec[i]}] at line {i + 1}: `{old[i].strip()[:100]}`"
                 for i in range(i1, i2) if osec[i] in released and old[i].strip() and not LINKREF.match(old[i])]
        hits += [f"adds a line inside released section [{nsec[j]}]: `{new[j].strip()[:100]}`"
                 for j in range(j1, j2) if nsec[j] in released and new[j].strip() and not LINKREF.match(new[j])]
    errs = [f"{label}: {describe(k)}" for k in sorted(dup_keys(new, released) - dup_keys(old, released), key=str)]
    if allow_released:
        return errs, [f"{label}: {h} (allowed by the {OVERRIDE} label)" for h in hits]
    hint = f" Move it under [Unreleased], or add the {OVERRIDE} label if the edit is intended."
    return errs + [f"{label}: {h}.{hint}" for h in hits], []


def gh_json(path):
    out = run(GH, "api", "--paginate", path).stdout
    dec, i, items = json.JSONDecoder(), 0, []
    while i < len(out):  # --paginate prints one JSON array per page back to back
        if out[i].isspace():
            i += 1
            continue
        page, i = dec.raw_decode(out, i)
        items += page
    return items


def open_prs():
    prs = gh_json(f"repos/{os.environ['REPO']}/pulls?state=open&base={os.environ['BASE_BRANCH']}&per_page=100")
    return [(p["number"], p["head"]["sha"], {l["name"] for l in p.get("labels", [])}) for p in prs]


def fetch_pr(n):
    run("git", "fetch", "-q", "--no-tags", "origin", f"+refs/pull/{n}/head:refs/remotes/pr/{n}")
    return f"refs/remotes/pr/{n}"


def check_peers(base, head, me, prs):
    errs, released = [], released_of(show(base) or [])
    mine = show(head) or []
    for n, _, _ in sorted(p for p in prs if p[0] < me):
        ref = fetch_pr(n)
        if not touches(base, ref):
            continue
        tree, conflicts = merge(head, ref)
        if CL in conflicts:
            errs.append(f"#{me}: {CL} conflicts with open #{n}; update this branch after #{n} merges")
            continue
        both = dup_keys(mine, released) | dup_keys(show(ref) or [], released)
        errs += [f"#{me}: together with open #{n}, {describe(k)}; update this branch after #{n} merges"
                 for k in sorted(dup_keys(show(tree) or [], released) - both, key=str)]
    return errs


def set_status(sha, errs):
    desc = f"{len(errs)} changelog problem(s) after the base branch moved" if errs else "Changelog still merges cleanly"
    args = [GH, "api", "-X", "POST", f"repos/{os.environ['REPO']}/statuses/{sha}",
            "-f", f"state={'failure' if errs else 'success'}", "-f", "context=changelog-guard/base", "-f", f"description={desc}"]
    if os.environ.get("RUN_URL"):
        args += ["-f", f"target_url={os.environ['RUN_URL']}"]
    run(*args)


def report(errs, warns):
    for w in warns:
        print(f"::warning title=changelog-guard::{w}")
    for e in errs:
        print(f"::error title=changelog-guard::{e}")
    if (summary := os.environ.get("GITHUB_STEP_SUMMARY")):
        with open(summary, "a") as f:
            f.write("### changelog-guard\n\n" + ("\n".join(f"- {x}" for x in errs + warns) or "No changelog problems.") + "\n")


def main():
    base, mode = os.environ["BASE"], os.environ.get("MODE", "pr")
    if mode == "pr":
        head, me = os.environ.get("HEAD_REF", "HEAD"), int(os.environ["PR_NUMBER"])
        if show(base) is None or not touches(base, head):
            return print(f"{CL} not changed by #{me}, nothing to check")
        allow = OVERRIDE in os.environ.get("LABELS", "").split(",")
        errs, warns = check_base(base, head, f"#{me}", allow)
        errs += check_peers(base, head, me, open_prs())
        report(errs, warns)
        sys.exit(1 if errs else 0)
    if show(base) is None:
        return print(f"no {CL} on the base branch, nothing to recheck")
    found = []
    for n, sha, labels in open_prs():
        ref = fetch_pr(n)
        if not touches(base, ref):
            continue
        errs, warns = check_base(base, ref, f"#{n}", OVERRIDE in labels)
        set_status(sha, errs)
        print(f"#{n}: {'failure' if errs else 'success'}")
        found += errs + warns
    # The problems belong to the PRs (their statuses carry them), so the push itself stays green.
    report([], found)


if __name__ == "__main__":
    main()
