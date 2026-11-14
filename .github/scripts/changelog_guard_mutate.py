#!/usr/bin/env python3
"""Mutate each clause of the guard's docstring, each regex and each API filter, one at a time, and
check that a self-test case fails for every mutant."""
import shutil, subprocess, sys, tempfile
from pathlib import Path

SRC = Path(__file__).resolve().parent
M = [
    # Docstring, pr mode: each rule
    ("conflict with the base fails", "    if CL in conflicts:\n        return [f\"{label}: {CL} conflicts with the base branch", "    if False:\n        return [f\"{label}: {CL} conflicts with the base branch"),
    ("released: changed or removed line", "for i in range(i1, i2) if osec[i] in released", "for i in range(i1, i2) if False"),
    ("released: added line", "for j in range(j1, j2) if nsec[j] in released", "for j in range(j1, j2) if False"),
    ("released: blank lines don't count", "if osec[i] in released and old[i].strip() and", "if osec[i] in released and"),
    ("released: blank lines don't count, new side", "if nsec[j] in released and new[j].strip() and", "if nsec[j] in released and"),
    ("released means already on the base", "released = sections(old), sections(new), released_of(old)", "released = sections(old), sections(new), released_of(new)"),
    ("Unreleased is not released", 'if s and s.lower() != "unreleased"}', "if s}"),
    ("override label", "    if allow_released:\n", "    if False:\n"),
    ("override label only covers released edits", "        return errs, [f\"{label}: {h} (allowed", "        return [], [f\"{label}: {h} (allowed"),
    ("override label makes them warnings", "(allowed by the {OVERRIDE} label)\" for h in hits]", "(allowed by the {OVERRIDE} label)\" for h in []]"),
    ("link references never count, old side", "and not LINKREF.match(old[i])]", "]"),
    ("link references never count, new side", "and not LINKREF.match(new[j])]", "]"),
    ("duplicate ## heading", 'k, h3 = ("section", m.group(1)), None', "continue"),
    ("duplicate ### heading in one section", 'k, h3 = ("heading", sec, m.group(1)), m.group(1)', "h3 = m.group(1)\n            continue"),
    ("duplicate entry", 'k = ("entry", sec, h3, norm(m.group(1)))', "continue"),
    ("entry duplicates are per heading", 'k = ("entry", sec, h3, norm(m.group(1)))', 'k = ("entry", sec, None, norm(m.group(1)))'),
    ("case ignored", '.strip().lower()', ".strip()"),
    ("spacing ignored", 're.sub(r"\\s+", " ", s)', "s"),
    ("duplicates already on the base aren't blamed", "dup_keys(new, released) - dup_keys(old, released)", "dup_keys(new, released)"),
    ("untouched changelog is not checked", "if show(base) is None or not touches(base, head):", "if show(base) is None:"),
    ("labels read from the API", "OVERRIDE in labels(me)", "False"),
    ("errors are ::error", 'print(f"::error title=changelog-guard::{e}")', 'print(f"::warning title=changelog-guard::{e}")'),
    ("pr exits 1 on errors", "sys.exit(1 if errs else 0)", "sys.exit(0)"),
    # Docstring, peers
    ("peer: only lower-numbered", "for p in prs if p[0] < me)", "for p in prs if p[0] != me)"),
    ("peer: only PRs that edit the changelog", "        if not touches(base, ref):\n            continue\n", ""),
    ("peer conflict", "        if CL in conflicts:\n            errs.append", "        if False:\n            errs.append"),
    ("peer duplicates", "for k in sorted(dup_keys(show(tree) or [], released) - both, key=str)]", "for k in []]"),
    ("peer duplicates the PRs bring alone aren't blamed", "dup_keys(show(tree) or [], released) - both", "dup_keys(show(tree) or [], released)"),
    ("peer message says the check re-runs by itself", 'again = f"this check re-runs by itself when #{n} is pushed, retargeted, merged or closed"', 'again = f"update this branch after #{n} merges"'),
    # Docstring, recheck and peers modes
    ("recheck covers only PRs that edit the changelog", "if touches(base, fetch_pr(n)) and", "if fetch_pr(n) and"),
    ("peers covers only higher-numbered PRs", "prs = [p for p in prs if p[0] > int(os.environ[\"PEER\"])]", "pass"),
    ("a queued or running job is left alone", '        if j["status"] != "completed":\n', '        if j["status"] == "in_progress":\n'),
    ("only the check job is re-run", 'if j["name"] == JOB and', "if"),
    ("runs that skipped the check are passed over", ' and j["conclusion"] != "skipped"]', "]"),
    ("older runs are looked at when the newest skipped", "        if not jobs:\n            continue\n", "        if not jobs:\n            break\n"),
    ("re-run the job, not the whole run", 'f"repos/{repo()}/actions/jobs/{j[\'id\']}/rerun"', 'f"repos/{repo()}/actions/runs/{r[\'id\']}/rerun"'),
    ("a PR with no check job gets a warning", '    return f"#{n}: no {JOB} job on', '    return None\n    return f"#{n}: no {JOB} job on'),
    ("a failure on one PR doesn't stop the loop", "        except (CmdError, KeyError, ValueError) as e:\n            warns.append", "        except ZeroDivisionError as e:\n            warns.append"),
    ("modes exit 0 when listing fails", "    except (CmdError, KeyError, ValueError) as e:\n        return report", "    except ZeroDivisionError as e:\n        return report"),
    # Regexes
    ("H3 strips trailing space", 'H3 = re.compile(r"^### +(.+?)\\s*$")', 'H3 = re.compile(r"^### +(.+)$")'),
    ("BULLET is top-level only", 'BULLET = re.compile(r"^[-*+]\\s+(.*\\S)")', 'BULLET = re.compile(r"^\\s*[-*+]\\s+(.*\\S)")'),
    ("LINKREF matches link references", 'LINKREF = re.compile(r"^\\[[^\\]]+\\]:\\s")', 'LINKREF = re.compile(r"^\\[[^\\]]+\\]:\\s{2}")'),
    ("H2 keys on the bracketed version only", 'H2 = re.compile(r"^## \\[([^\\]]+)\\]")', 'H2 = re.compile(r"^## (.+)")'),
    # API filters and paging
    ("open PRs only", "pulls?state=open&base=", "pulls?state=all&base="),
    ("PRs into the same base only", "&base={os.environ['BASE_BRANCH']}", ""),
    ("pull_request runs only", "?event=pull_request&head_sha=", "?head_sha="),
    ("runs on the PR's head commit only", "&head_sha={sha}&per_page=20", "&per_page=20"),
    ("pagination", "        page, i = dec.raw_decode(out, i)\n        items += page", "        items += json.loads(out[i:])\n        break"),
    ("three-dot diff range", 'f"{base}...{head}"', 'f"{base}..{head}"'),
]


def main():
    src = (SRC / "changelog_guard.py").read_text()
    survived = []
    for name, old, new in M:
        if src.count(old) != 1:
            sys.exit(f"mutant '{name}': pattern found {src.count(old)} times")
        with tempfile.TemporaryDirectory() as d:
            shutil.copy(SRC / "changelog_guard_test.py", d)
            Path(d, "changelog_guard.py").write_text(src.replace(old, new))
            p = subprocess.run([sys.executable, "-I", "changelog_guard_test.py"], cwd=d, capture_output=True, text=True)
        killed = p.returncode != 0
        print(f"{'killed  ' if killed else 'SURVIVED'} {name}")
        survived += [] if killed else [name]
    print(f"{len(M) - len(survived)} of {len(M)} mutants killed")
    sys.exit(1 if survived else 0)


if __name__ == "__main__":
    main()
