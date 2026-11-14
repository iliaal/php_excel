#!/usr/bin/env python3
"""Delete each rule the guard's docstring lists, one at a time, and check a self-test case fails."""
import shutil, subprocess, sys, tempfile
from pathlib import Path

SRC = Path(__file__).resolve().parent
M = [
    ("base conflict", "    if CL in conflicts:\n        return [f\"{label}: {CL} conflicts with the base branch", "    if False:\n        return [f\"{label}: {CL} conflicts with the base branch"),
    ("released: changed/removed line", "for i in range(i1, i2) if osec[i] in released", "for i in range(i1, i2) if False"),
    ("released: added line", "for j in range(j1, j2) if nsec[j] in released", "for j in range(j1, j2) if False"),
    ("LINKREF exclusion, old side", "and not LINKREF.match(old[i])]", "]"),
    ("LINKREF exclusion, new side", "and not LINKREF.match(new[j])]", "]"),
    ("Unreleased is not released", 'if s and s.lower() != "unreleased"}', "if s}"),
    ("override label", "    if allow_released:\n", "    if False:\n"),
    ("duplicate ## heading", 'k, h3 = ("section", m.group(1)), None', "continue"),
    ("duplicate ### heading", 'k, h3 = ("heading", sec, m.group(1)), m.group(1)', "h3 = m.group(1)\n            continue"),
    ("duplicate entry", 'k = ("entry", sec, h3, norm(m.group(1)))', "continue"),
    ("norm lowercasing", '.strip().lower()', ".strip()"),
    ("norm whitespace", 're.sub(r"\\s+", " ", s)', "s"),
    ("base-duplicate subtraction", "dup_keys(new, released) - dup_keys(old, released)", "dup_keys(new, released)"),
    ("peer: only lower-numbered", "for p in prs if p[0] < me)", "for p in prs if p[0] != me)"),
    ("peer conflict", "        if CL in conflicts:\n            errs.append", "        if False:\n            errs.append"),
    ("peer duplicates", "for k in sorted(dup_keys(show(tree) or [], released) - both, key=str)]", "for k in []]"),
    ("peer duplicate subtraction", "dup_keys(show(tree) or [], released) - both", "dup_keys(show(tree) or [], released)"),
    ("pagination", "        page, i = dec.raw_decode(out, i)\n        items += page", "        items += json.loads(out[i:])\n        break"),
    ("three-dot diff range", 'f"{base}...{head}"', 'f"{base}..{head}"'),
    ("base filter on PR listing", "&base={os.environ['BASE_BRANCH']}", ""),
    ("labels read from the API", "OVERRIDE in labels(me)", "False"),
    ("pr skips untouched changelog", "if show(base) is None or not touches(base, head):", "if show(base) is None:"),
    ("recheck skips PRs not touching the changelog", "if touches(base, fetch_pr(n)) and", "if fetch_pr(n) and"),
    ("recheck keeps going past a failure", "        except (CmdError, KeyError, ValueError) as e:\n            warns.append", "        except ZeroDivisionError as e:\n            warns.append"),
    ("recheck exits 0 when listing fails", "    except (CmdError, KeyError, ValueError) as e:\n        return report", "    except ZeroDivisionError as e:\n        return report"),
    ("rerun skips runs still going", 'if r["status"] != "completed":', "if False:"),
    ("rerun reports a PR with no run", "    if not runs:\n        return f", "    if not runs:\n        return None\n        return f"),
    ("peers: only higher-numbered", "prs = [p for p in prs if p[0] > int(os.environ[\"PEER\"])]", "pass"),
    ("pr exits 1 on errors", "sys.exit(1 if errs else 0)", "sys.exit(0)"),
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
