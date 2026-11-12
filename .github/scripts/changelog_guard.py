#!/usr/bin/env python3
"""Fail PRs that edit released CHANGELOG sections or collide with other open PRs on the changelog.

Rules (#ai PR reviews): 2026-11-12 V2/V3, 2026-11-11 V2/V3, 2026-11-10 V3, 2026-11-09 V3.
Released section = any `## [x]` heading other than Unreleased that exists on the default branch.
Sections new in the PR (e.g. a fresh version block in repos without [Unreleased]) are editable.

Env: MODE=pr|push, BASE (e.g. origin/main), HEAD_REF (default HEAD), PR_NUMBER, REPO (owner/name),
     CHANGELOG (default CHANGELOG.md), ALLOW_UNRELEASED_EDITS=1 to allow rewording existing lines.
"""
import difflib, os, re, subprocess, sys

CL = os.environ.get("CHANGELOG", "CHANGELOG.md")
ALLOW_EDITS = os.environ.get("ALLOW_UNRELEASED_EDITS") == "1"
H2 = re.compile(r"^## \[([^\]]+)\]")
H3 = re.compile(r"^### +(.+?)\s*$")
BULLET = re.compile(r"^\s*[-*+]\s+(.*\S)")
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


def norm(s):
    return re.sub(r"\s+", " ", s).strip().lower()


def dup_keys(lines, released):
    """Duplicate ## headings anywhere; duplicate ### headings and bullets inside editable sections."""
    seen, dups = set(), set()
    for sec, l in zip(sections(lines), lines):
        if (m := H2.match(l)):
            k = ("section", m.group(1))
        elif sec in released:
            continue
        elif (m := H3.match(l)):
            k = ("heading", sec, m.group(1))
        elif (m := BULLET.match(l)):
            k = ("entry", sec, norm(m.group(1)))
        else:
            continue
        (dups if k in seen else seen).add(k)
    return dups


def describe(k):
    if k[0] == "section":
        return f"`## [{k[1]}]` appears twice"
    if k[0] == "heading":
        return f"`### {k[2]}` appears twice under [{k[1]}] (put the bullet under the existing heading)"
    return f"entry `{k[2][:80]}` appears twice under [{k[1]}]"


def check_base(base, head, label):
    """Diff the default branch against the merge result, as the review rules specify."""
    tree, conflicts = merge(base, head)
    if CL in conflicts:
        return [f"{label}: {CL} conflicts with {base}. Rebase and put entries under the existing headings."]
    old, new = show(base) or [], show(tree)
    if new is None:
        return []
    osec, nsec = sections(old), sections(new)
    released = {s for s in osec if s and s.lower() != "unreleased"}
    known_h3 = {m.group(1) for l in old if (m := H3.match(l))}
    old_entries = {norm(m.group(1)) for l in old if (m := BULLET.match(l))}
    errs = []
    for tag, i1, i2, j1, j2 in difflib.SequenceMatcher(None, old, new, autojunk=False).get_opcodes():
        if tag == "equal":
            continue
        for i in range(i1, i2):
            l = old[i]
            if not l.strip() or LINKREF.match(l):
                continue
            if osec[i] in released:
                errs.append(f"{label}: changes released section [{osec[i]}] at {base}:{CL}:{i+1}: `{l.strip()}`")
            elif not ALLOW_EDITS:
                errs.append(f"{label}: removes or rewrites an existing line at {base}:{CL}:{i+1}: `{l.strip()}`")
        for j in range(j1, j2):
            l = new[j]
            if not l.strip() or LINKREF.match(l):
                continue
            if nsec[j] in released:
                errs.append(f"{label}: adds a line inside released section [{nsec[j]}] (merged {CL}:{j+1}): `{l.strip()}`")
            if (m := H3.match(l)) and m.group(1) not in known_h3:
                errs.append(f"{label}: new heading `### {m.group(1)}` is not used anywhere in {CL}")
            if (m := BULLET.match(l)) and norm(m.group(1)) in old_entries:
                errs.append(f"{label}: entry duplicates one already in {CL}: `{m.group(1)[:80]}`")
    for k in dup_keys(new, released) - dup_keys(old, released):
        errs.append(f"{label}: {describe(k)}")
    return errs


def open_prs(base_branch):
    p = run("gh", "api", "--paginate", f"repos/{os.environ['REPO']}/pulls?state=open&base={base_branch}&per_page=100",
            "-q", ".[] | .number")
    return [int(n) for n in p.stdout.split()]


def fetch_pr(n):
    run("git", "fetch", "-q", "--no-tags", "origin", f"+refs/pull/{n}/head:refs/remotes/pr/{n}")
    return f"refs/remotes/pr/{n}"


def check_peers(base, head, me):
    """Only PRs opened earlier count, so the later of two colliding PRs is the one that waits."""
    errs, released = [], {s for s in sections(show(base) or []) if s and s.lower() != "unreleased"}
    mine = show(head) or []
    for n in sorted(x for x in open_prs(base.split("/", 1)[1]) if x < me):
        ref = fetch_pr(n)
        if not touches(base, ref):
            continue
        tree, conflicts = merge(head, ref)
        if CL in conflicts:
            errs.append(f"#{me}: {CL} conflicts with open #{n}. Open this after #{n} merges, then rebase.")
            continue
        if conflicts:
            print(f"::warning title=changelog-guard::#{me} also conflicts with #{n} in: {', '.join(conflicts)}")
        both = dup_keys(mine, released) | dup_keys(show(ref) or [], released)
        for k in dup_keys(show(tree) or [], released) - both:
            errs.append(f"#{me}: combined with open #{n}, {describe(k)}. Open this after #{n} merges.")
    return errs


def main():
    base, mode = os.environ["BASE"], os.environ.get("MODE", "pr")
    if show(base) is None and show(os.environ.get("HEAD_REF", "HEAD")) is None:
        return print(f"no {CL}, skipping")
    if mode == "pr":
        head, me = os.environ.get("HEAD_REF", "HEAD"), int(os.environ["PR_NUMBER"])
        errs = check_base(base, head, f"#{me}") + check_peers(base, head, me) if touches(base, head) else []
    else:  # push to default branch: recheck every open PR after a release commit
        errs = []
        for n in open_prs(base.split("/", 1)[1]):
            ref = fetch_pr(n)
            if touches(base, ref):
                errs += check_base(base, ref, f"#{n}")
    for e in errs:
        print(f"::error title=changelog-guard::{e}")
    if (summary := os.environ.get("GITHUB_STEP_SUMMARY")):
        with open(summary, "a") as f:
            f.write("### changelog-guard\n\n" + ("\n".join(f"- {e}" for e in errs) or "No changelog problems.") + "\n")
    sys.exit(1 if errs else 0)


main()
