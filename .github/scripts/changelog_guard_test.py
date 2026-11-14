#!/usr/bin/env python3
"""Fixture tests for changelog_guard.py.

Each case builds a throwaway origin repo with refs/pull/N/head refs and a stub `gh` that serves the
open-PR listing (split over two pages and filtered by base branch, like the API), each PR's labels and
its latest workflow run, and records re-run requests. The guard then runs end to end: merge-tree, the
PR listing, the refs/pull fetch and the re-run call all run for real against the fixture.
"""
import json, os, subprocess, sys, tempfile, textwrap, unittest
from pathlib import Path
from urllib.parse import parse_qs, urlsplit

GUARD = Path(__file__).with_name("changelog_guard.py").resolve()
BASE_CL = """\
# Changelog

## [Unreleased]

### Added
- Existing feature

### Fixed
- Existing fix

## [1.0.0] - 2026-01-01

### Added
- First release

[Unreleased]: https://example.com/compare/v1.0.0...HEAD
[1.0.0]: https://example.com/releases/v1.0.0
"""
STUB = r'''#!/usr/bin/env python3
import json, os, sys
from urllib.parse import parse_qs, urlsplit
args = sys.argv[1:]
state = json.load(open(os.environ["STUB_STATE"]))
path = [a for a in args if a.startswith("repos/")][0]
u = urlsplit(path)
q = {k: v[0] for k, v in parse_qs(u.query).items()}
parts = u.path.split("/")
if "-X" in args:
    with open(os.environ["STUB_LOG"], "a") as f:
        f.write(u.path + "\n")
    print("{}")
elif parts[3] == "pulls":
    prs = [{"number": p["number"], "head": {"sha": p["sha"]}} for p in state["prs"]
           if p["state"] == q["state"] and p["base"] == q["base"]]
    half = len(prs) // 2
    print(json.dumps(prs[:half]) + "\n" + json.dumps(prs[half:]))  # two pages, like gh api --paginate
elif parts[3] == "issues":
    p = [p for p in state["prs"] if p["number"] == int(parts[4])][0]
    print(json.dumps([{"name": l} for l in p["labels"]]))
elif parts[3] == "actions":
    runs = [{"id": 1000 + p["number"], "status": p.get("run", "completed")} for p in state["prs"]
            if p["sha"] == q["head_sha"] and p.get("run", "completed") != "none"]
    print(json.dumps({"workflow_runs": runs[:1]}))
else:
    sys.exit(f"unexpected call {args}")
'''


def sh(cwd, *cmd):
    return subprocess.run(cmd, cwd=cwd, check=True, capture_output=True, text=True).stdout.strip()


class Fixture:
    def __init__(self, tmp, base_cl=BASE_CL):
        self.root = Path(tmp)
        self.origin, self.work = self.root / "origin.git", self.root / "work"
        sh(self.root, "git", "init", "-q", "--bare", "-b", "main", str(self.origin))
        sh(self.root, "git", "clone", "-q", str(self.origin), str(self.work))
        for k, v in (("user.name", "t"), ("user.email", "t@example.com"), ("commit.gpgsign", "false")):
            sh(self.work, "git", "config", k, v)
        self.commit("main", base_cl, "base", new=True)
        self.prs, self.log, self.state = [], self.root / "calls.log", self.root / "state.json"
        stub = self.root / "gh"
        stub.write_text(STUB)
        stub.chmod(0o755)
        self.gh = str(stub)

    def commit(self, branch, text, msg, new=False, start="main", path="CHANGELOG.md"):
        if not new:
            sh(self.work, "git", "checkout", "-q", "-B", branch, f"origin/{start}")
        (self.work / path).write_text(textwrap.dedent(text))
        sh(self.work, "git", "add", "-A")
        sh(self.work, "git", "commit", "-q", "-m", msg)
        sh(self.work, "git", "push", "-q", "origin", f"HEAD:refs/heads/{branch}")
        return sh(self.work, "git", "rev-parse", "HEAD")

    def pr(self, n, text, labels=(), base="main", path="CHANGELOG.md", push_ref=True, run="completed"):
        sha = self.commit(f"pr{n}", text, f"pr {n}", start=base, path=path)
        if push_ref:
            sh(self.work, "git", "push", "-q", "origin", f"HEAD:refs/pull/{n}/head")
        self.prs.append({"number": n, "sha": sha, "labels": list(labels), "base": base, "state": "open", "run": run})
        return sha

    def get(self, n):
        return [p for p in self.prs if p["number"] == n][0]

    def run(self, mode, n=None, **extra):
        self.state.write_text(json.dumps({"prs": self.prs}))
        sh(self.work, "git", "fetch", "-q", "origin")
        env = dict(os.environ, MODE=mode, BASE="origin/main", BASE_BRANCH="main", REPO="o/r", GH=self.gh,
                   STUB_STATE=str(self.state), STUB_LOG=str(self.log))
        env.update(extra)
        env.pop("GITHUB_STEP_SUMMARY", None)
        if n is not None:
            env.update(PR_NUMBER=str(n), HEAD_REF=f"origin/pr{n}")
        return subprocess.run([sys.executable, str(GUARD)], cwd=self.work, env=env, capture_output=True, text=True)

    def reruns(self):
        if not self.log.exists():
            return set()
        return {int(l.split("/")[-2]) - 1000 for l in self.log.read_text().splitlines() if l.endswith("/rerun")}


def add_unreleased(entry, heading="### Fixed", base=BASE_CL):
    return base.replace(f"{heading}\n", f"{heading}\n{entry}\n", 1)


class GuardTest(unittest.TestCase):
    base_cl = BASE_CL

    def setUp(self):
        self._tmp = tempfile.TemporaryDirectory()
        self.f = Fixture(self._tmp.name, self.base_cl)

    def tearDown(self):
        self._tmp.cleanup()

    def assertPass(self, p):
        self.assertEqual(p.returncode, 0, p.stdout + p.stderr)

    def assertFail(self, p, needle):
        self.assertEqual(p.returncode, 1, p.stdout + p.stderr)
        self.assertIn(needle, p.stdout)

    # Allowed edits
    def test_entry_under_unreleased_passes(self):
        self.f.pr(1, add_unreleased("- New fix"))
        self.assertPass(self.f.run("pr", 1))

    def test_rewording_in_unreleased_passes(self):
        self.f.pr(1, BASE_CL.replace("- Existing fix\n", "- Existing fix, reworded\n"))
        self.assertPass(self.f.run("pr", 1))

    def test_removal_in_unreleased_passes(self):
        self.f.pr(1, BASE_CL.replace("- Existing fix\n", ""))
        self.assertPass(self.f.run("pr", 1))

    def test_new_heading_passes(self):
        self.f.pr(1, BASE_CL.replace("- Existing fix\n", "- Existing fix\n\n### Security\n- New\n"))
        self.assertPass(self.f.run("pr", 1))

    def test_release_pr_updating_link_references_passes(self):
        released = (BASE_CL.replace("## [Unreleased]\n", "## [Unreleased]\n\n## [1.1.0] - 2026-02-01\n", 1)
                    .replace("compare/v1.0.0...HEAD", "compare/v1.1.0...HEAD")
                    .replace("[1.0.0]: ", "[1.1.0]: https://example.com/releases/v1.1.0\n[1.0.0]: "))
        self.f.pr(1, released)
        self.assertPass(self.f.run("pr", 1))

    def test_untouched_changelog_skips_even_after_base_moved(self):
        self.f.pr(1, "x", path="x.txt")
        self.f.commit("main", add_unreleased("- Landed meanwhile"), "main moves")
        p = self.f.run("pr", 1)
        self.assertPass(p)
        self.assertIn("nothing to check", p.stdout)

    def test_duplicates_already_on_base_are_not_blamed(self):
        dup = BASE_CL.replace("- Existing fix\n", "- Existing fix\n- Existing fix\n")
        self.f.commit("main", dup, "base has a duplicate")
        self.f.pr(1, add_unreleased("- New fix", base=dup))
        self.f.pr(2, add_unreleased("- Other", heading="### Added", base=dup))
        self.assertPass(self.f.run("pr", 1))
        self.assertPass(self.f.run("pr", 2))

    # Rejected edits
    def test_released_edit_fails_and_label_overrides(self):
        self.f.pr(1, BASE_CL.replace("- First release", "- First release, reworded"))
        self.assertFail(self.f.run("pr", 1), "changes released section [1.0.0]")
        self.f.get(1)["labels"] = ["other", "changelog-override"]
        p = self.f.run("pr", 1)
        self.assertPass(p)
        self.assertIn("::warning", p.stdout)

    def test_line_added_to_released_section_fails(self):
        self.f.pr(1, BASE_CL.replace("- First release\n", "- First release\n- Late addition\n"))
        self.assertFail(self.f.run("pr", 1), "adds a line inside released section [1.0.0]")

    def test_conflict_with_base_fails(self):
        self.f.pr(1, BASE_CL.replace("- Existing fix", "- Fix A"))
        self.f.commit("main", BASE_CL.replace("- Existing fix", "- Fix B"), "main edits the same line")
        self.assertFail(self.f.run("pr", 1), "conflicts with the base branch")

    def test_duplicate_heading_fails(self):
        self.f.pr(1, BASE_CL.replace("- Existing fix\n", "- Existing fix\n\n### Fixed\n- Another\n"))
        self.assertFail(self.f.run("pr", 1), "`### Fixed` appears twice")

    def test_duplicate_version_heading_fails(self):
        self.f.pr(1, BASE_CL.replace("## [Unreleased]\n", "## [Unreleased]\n\n## [1.0.0] - 2026-02-01\n", 1))
        self.assertFail(self.f.run("pr", 1), "`## [1.0.0]` appears twice")

    def test_duplicate_entry_fails_ignoring_case(self):
        self.f.pr(1, add_unreleased("- existing   FIX"))
        self.assertFail(self.f.run("pr", 1), "appears twice under [Unreleased]")

    # Peers
    def test_later_colliding_pr_fails_earlier_passes(self):
        self.f.pr(1, add_unreleased("- Fix A"))
        self.f.pr(2, add_unreleased("- Fix B"))
        self.f.pr(3, "x", path="x.txt")  # a third PR keeps both listing pages non-empty
        self.assertPass(self.f.run("pr", 1))
        self.assertFail(self.f.run("pr", 2), "conflicts with open #1")

    def test_peers_that_merge_cleanly_into_a_duplicate_heading_fail(self):
        self.f.pr(1, BASE_CL.replace("### Fixed\n", "### Security\n- A\n\n### Fixed\n"))
        self.f.pr(2, BASE_CL.replace("- Existing fix\n", "- Existing fix\n\n### Security\n- B\n"))
        self.assertPass(self.f.run("pr", 1))
        self.assertFail(self.f.run("pr", 2), "together with open #1, `### Security` appears twice")

    def test_peer_into_another_base_branch_is_ignored(self):
        sh(self.f.work, "git", "push", "-q", "origin", "origin/main:refs/heads/release")
        self.f.pr(1, add_unreleased("- Fix A"), base="release")
        self.f.pr(2, add_unreleased("- Fix B"))
        self.assertPass(self.f.run("pr", 2))

    # Every failure clears through its remedy, with the PR's own check as the one signal
    def test_release_push_reruns_affected_pr_then_label_clears_it(self):
        self.f.pr(1, add_unreleased("- Pending fix"))
        self.f.pr(2, "x", path="x.txt")
        self.assertPass(self.f.run("pr", 1))
        released = BASE_CL.replace("## [Unreleased]\n", "## [Unreleased]\n\n## [1.1.0] - 2026-02-01\n", 1)
        self.f.commit("main", released, "Release 1.1.0")
        self.assertPass(self.f.run("recheck"))
        self.assertEqual(self.f.reruns(), {1})  # #2 doesn't touch the changelog
        self.assertFail(self.f.run("pr", 1), "released section [1.1.0]")  # the re-run goes red
        self.f.get(1)["labels"] = ["changelog-override"]
        self.assertPass(self.f.run("pr", 1))  # the labeled run is green on the same check

    def test_closing_a_peer_reruns_later_prs_and_clears_them(self):
        self.f.pr(1, add_unreleased("- Fix A"))
        self.f.pr(2, add_unreleased("- Fix B"))
        self.f.pr(3, add_unreleased("- Fix C"))
        self.assertFail(self.f.run("pr", 2), "conflicts with open #1")
        self.f.get(1)["state"] = "closed"
        self.assertPass(self.f.run("peers", PEER="1"))
        self.assertEqual(self.f.reruns(), {2, 3})
        self.assertPass(self.f.run("pr", 2))

    def test_closing_a_peer_leaves_lower_numbered_prs_alone(self):
        self.f.pr(1, add_unreleased("- Fix A"))
        self.f.pr(2, add_unreleased("- Fix B", heading="### Added"))
        self.f.pr(3, BASE_CL.replace("# Changelog\n", "# Changelog\n\nIntro.\n"))
        self.f.get(2)["state"] = "closed"
        self.assertPass(self.f.run("peers", PEER="2"))
        self.assertEqual(self.f.reruns(), {3})

    # Recheck never fails and never stops at the first PR
    def test_recheck_survives_a_failed_fetch_and_a_missing_run(self):
        self.f.pr(1, add_unreleased("- Fix A"))
        self.f.pr(2, add_unreleased("- Fix B", heading="### Added"), push_ref=False)  # fetch fails
        self.f.pr(3, BASE_CL.replace("# Changelog\n", "# Changelog\n\nIntro.\n"))
        self.f.pr(4, BASE_CL.replace("- First release", "- First"), run="none")
        self.f.pr(5, BASE_CL.replace("[1.0.0]: ", "[1.0.0]:  "), run="in_progress")
        p = self.f.run("recheck")
        self.assertPass(p)
        self.assertEqual(self.f.reruns(), {1, 3})
        self.assertIn("#2: could not re-run", p.stdout)
        self.assertIn("#4: no changelog-guard.yml run", p.stdout)

    def test_recheck_exits_zero_when_listing_fails(self):
        self.f.pr(1, add_unreleased("- Fix A"))
        p = self.f.run("recheck", GH="/bin/false")
        self.assertPass(p)
        self.assertIn("could not list open PRs", p.stdout)


if __name__ == "__main__":
    unittest.main(verbosity=2)
