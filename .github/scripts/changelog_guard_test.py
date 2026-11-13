#!/usr/bin/env python3
"""Fixture tests for changelog_guard.py.

Each case builds a throwaway origin repo with refs/pull/N/head refs and a stub `gh` that serves the
open-PR listing and records status writes, then runs the guard end to end: merge-tree, the PR
listing, the refs/pull fetch and the status call all run for real against the fixture.
"""
import json, os, subprocess, sys, tempfile, textwrap, unittest
from pathlib import Path

GUARD = Path(__file__).with_name("changelog_guard.py").resolve()
BASE_CL = """\
# Changelog

## [Unreleased]

### Fixed
- Existing fix

## [1.0.0] - 2026-01-01

### Added
- First release
"""
STUB = """\
#!/usr/bin/env python3
import json, os, sys
args = sys.argv[1:]
if "-X" in args:
    with open(os.environ["STUB_LOG"], "a") as f:
        f.write(json.dumps(args) + "\\n")
    print("{}")
else:
    print(open(os.environ["STUB_PRS"]).read())
"""


def sh(cwd, *cmd):
    return subprocess.run(cmd, cwd=cwd, check=True, capture_output=True, text=True).stdout.strip()


class Fixture:
    def __init__(self, tmp):
        self.root = Path(tmp)
        self.origin, self.work = self.root / "origin.git", self.root / "work"
        sh(self.root, "git", "init", "-q", "--bare", "-b", "main", str(self.origin))
        sh(self.root, "git", "clone", "-q", str(self.origin), str(self.work))
        for k, v in (("user.name", "t"), ("user.email", "t@example.com"), ("commit.gpgsign", "false")):
            sh(self.work, "git", "config", k, v)
        self.commit("main", BASE_CL, "base", new=True)
        self.prs, self.log = [], self.root / "status.log"
        stub = self.root / "gh"
        stub.write_text(STUB)
        stub.chmod(0o755)
        self.gh = str(stub)

    def commit(self, branch, text, msg, new=False, start="main"):
        if not new:
            sh(self.work, "git", "checkout", "-q", "-B", branch, f"origin/{start}" if start else branch)
        (self.work / "CHANGELOG.md").write_text(textwrap.dedent(text))
        sh(self.work, "git", "add", "-A")
        sh(self.work, "git", "commit", "-q", "-m", msg)
        sh(self.work, "git", "push", "-q", "origin", f"HEAD:refs/heads/{branch}")
        return sh(self.work, "git", "rev-parse", "HEAD")

    def pr(self, n, text, labels=()):
        sha = self.commit(f"pr{n}", text, f"pr {n}")
        sh(self.work, "git", "push", "-q", "origin", f"HEAD:refs/pull/{n}/head")
        self.prs.append({"number": n, "head": {"sha": sha}, "labels": [{"name": l} for l in labels]})
        return sha

    def run(self, mode, n=None, labels=""):
        (self.root / "prs.json").write_text(json.dumps(self.prs))
        sh(self.work, "git", "fetch", "-q", "origin")
        env = dict(os.environ, MODE=mode, BASE="origin/main", BASE_BRANCH="main", REPO="o/r", GH=self.gh,
                   STUB_PRS=str(self.root / "prs.json"), STUB_LOG=str(self.log), LABELS=labels)
        env.pop("GITHUB_STEP_SUMMARY", None)
        if n is not None:
            env.update(PR_NUMBER=str(n), HEAD_REF=f"origin/pr{n}")
        return subprocess.run([sys.executable, str(GUARD)], cwd=self.work, env=env, capture_output=True, text=True)

    def statuses(self):
        if not self.log.exists():
            return {}
        out = {}
        for line in self.log.read_text().splitlines():
            a = json.loads(line)
            fields = dict(x.split("=", 1) for x in a if "=" in x and not x.startswith("repos/"))
            out[a[a.index("POST") + 1].rsplit("/", 1)[1]] = fields["state"]
        return out


def add_unreleased(entry, heading="### Fixed", base=BASE_CL):
    return base.replace(f"## [Unreleased]\n\n{heading}\n", f"## [Unreleased]\n\n{heading}\n{entry}\n", 1)


class GuardTest(unittest.TestCase):
    def setUp(self):
        self._tmp = tempfile.TemporaryDirectory()
        self.f = Fixture(self._tmp.name)

    def tearDown(self):
        self._tmp.cleanup()

    def assertPass(self, p):
        self.assertEqual(p.returncode, 0, p.stdout + p.stderr)

    def assertFail(self, p, needle):
        self.assertEqual(p.returncode, 1, p.stdout + p.stderr)
        self.assertIn(needle, p.stdout)

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
        self.f.pr(1, BASE_CL.replace("### Fixed\n- Existing fix\n", "### Fixed\n- Existing fix\n\n### Security\n- New\n"))
        self.assertPass(self.f.run("pr", 1))

    def test_released_edit_fails_and_label_overrides(self):
        self.f.pr(1, BASE_CL.replace("- First release", "- First release, reworded"))
        self.assertFail(self.f.run("pr", 1), "released section [1.0.0]")
        p = self.f.run("pr", 1, labels="other,changelog-override")
        self.assertPass(p)
        self.assertIn("::warning", p.stdout)

    def test_duplicate_heading_fails(self):
        self.f.pr(1, BASE_CL.replace("- Existing fix\n", "- Existing fix\n\n### Fixed\n- Another\n"))
        self.assertFail(self.f.run("pr", 1), "`### Fixed` appears twice")

    def test_untouched_changelog_skips(self):
        sh(self.f.work, "git", "checkout", "-q", "-B", "pr1", "origin/main")
        (self.f.work / "x.txt").write_text("x")
        sh(self.f.work, "git", "add", "-A")
        sh(self.f.work, "git", "commit", "-q", "-m", "x")
        sh(self.f.work, "git", "push", "-q", "origin", "HEAD:refs/heads/pr1")
        p = self.f.run("pr", 1)
        self.assertPass(p)
        self.assertIn("nothing to check", p.stdout)

    def test_later_colliding_pr_fails_earlier_passes(self):
        self.f.pr(1, add_unreleased("- Fix A"))
        self.f.pr(2, add_unreleased("- Fix B"))
        self.assertPass(self.f.run("pr", 1))
        self.assertFail(self.f.run("pr", 2), "conflicts with open #1")

    def test_recheck_after_release_marks_pr_and_push_stays_green(self):
        sha = self.f.pr(1, add_unreleased("- Pending fix"))
        ok = self.f.pr(2, BASE_CL.replace("# Changelog\n", "# Changelog\n\nIntro.\n"))
        released = BASE_CL.replace("## [Unreleased]\n", "## [Unreleased]\n\n## [1.1.0] - 2026-02-01\n", 1)
        self.f.commit("main", released, "Release 1.1.0")
        p = self.f.run("recheck")
        self.assertPass(p)
        self.assertEqual(self.f.statuses(), {sha: "failure", ok: "success"})


if __name__ == "__main__":
    unittest.main(verbosity=2)
