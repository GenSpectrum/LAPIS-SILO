from __future__ import annotations

from pathlib import Path
import subprocess
import sys
import textwrap


REPO_ROOT = Path(__file__).resolve().parents[2]
CHECK_SCRIPT = REPO_ROOT / "buildScripts" / "check_trailing_commas.py"


def run_check(tmp_path: Path, source: str) -> subprocess.CompletedProcess[str]:
   cpp_file = tmp_path / "sample.cpp"
   cpp_file.write_text(textwrap.dedent(source))
   return subprocess.run(
      [sys.executable, str(CHECK_SCRIPT), str(cpp_file)],
      capture_output=True,
      text=True,
      check=False,
   )


def test_reports_missing_trailing_comma_in_multiline_initializer(tmp_path: Path) -> None:
   result = run_check(
      tmp_path,
      """
      struct Config {
         int first;
         int second;
      };

      Config config{
         .first = 1,
         .second = 2
      };
      """,
   )

   assert result.returncode == 1
   assert "multi-line braced initializer should end with a trailing comma" in result.stderr


def test_accepts_multiline_initializer_with_trailing_comma(tmp_path: Path) -> None:
   result = run_check(
      tmp_path,
      """
      struct Config {
         int first;
         int second;
      };

      Config config{
         .first = 1,
         .second = 2,
      };
      """,
   )

   assert result.returncode == 0
   assert result.stderr == ""


def test_diff_base_checks_only_changed_lines_with_relative_path(tmp_path: Path) -> None:
   cpp_file = tmp_path / "sample.cpp"
   cpp_file.write_text(
      textwrap.dedent(
         """
         struct Config {
            int first;
            int second;
         };

         Config config{
            .first = 1,
            .second = 2,
         };
         """
      )
   )
   subprocess.run(["git", "init"], cwd=tmp_path, check=True, capture_output=True, text=True)
   subprocess.run(["git", "config", "user.name", "Test User"], cwd=tmp_path, check=True)
   subprocess.run(["git", "config", "user.email", "test@example.com"], cwd=tmp_path, check=True)
   subprocess.run(["git", "add", "sample.cpp"], cwd=tmp_path, check=True)
   subprocess.run(["git", "commit", "-m", "base"], cwd=tmp_path, check=True, capture_output=True, text=True)

   cpp_file.write_text(
      textwrap.dedent(
         """
         struct Config {
            int first;
            int second;
         };

         Config config{
            .first = 1,
            .second = 2
         };
         """
      )
   )
   result = subprocess.run(
      [sys.executable, str(CHECK_SCRIPT), "--diff-base", "HEAD", "sample.cpp"],
      cwd=tmp_path,
      capture_output=True,
      text=True,
      check=False,
   )

   assert result.returncode == 1
   assert "sample.cpp:9:14" in result.stderr


def test_diff_base_checks_staged_only_changes(tmp_path: Path) -> None:
   cpp_file = tmp_path / "sample.cpp"
   cpp_file.write_text(
      textwrap.dedent(
         """
         struct Config {
            int first;
            int second;
         };

         Config config{
            .first = 1,
            .second = 2,
         };
         """
      )
   )
   subprocess.run(["git", "init"], cwd=tmp_path, check=True, capture_output=True, text=True)
   subprocess.run(["git", "config", "user.name", "Test User"], cwd=tmp_path, check=True)
   subprocess.run(["git", "config", "user.email", "test@example.com"], cwd=tmp_path, check=True)
   subprocess.run(["git", "add", "sample.cpp"], cwd=tmp_path, check=True)
   subprocess.run(["git", "commit", "-m", "base"], cwd=tmp_path, check=True, capture_output=True, text=True)

   cpp_file.write_text(
      textwrap.dedent(
         """
         struct Config {
            int first;
            int second;
         };

         Config config{
            .first = 1,
            .second = 2
         };
         """
      )
   )
   subprocess.run(["git", "add", "sample.cpp"], cwd=tmp_path, check=True)
   result = subprocess.run(
      [sys.executable, str(CHECK_SCRIPT), "--diff-base", "HEAD", "sample.cpp"],
      cwd=tmp_path,
      capture_output=True,
      text=True,
      check=False,
   )

   assert result.returncode == 1
   assert "sample.cpp:9:14" in result.stderr


def test_ignores_non_initializer_braces(tmp_path: Path) -> None:
   result = run_check(
      tmp_path,
      """
      namespace example {

      struct Config {
         int first;
         int second;
      };

      void run(bool ready) {
         if (ready) {
            const auto value = 1;
            (void)value;
         }
      }

      }  // namespace example
      """,
   )

   assert result.returncode == 0
   assert result.stderr == ""


def test_ignores_braces_inside_string_literals(tmp_path: Path) -> None:
   result = run_check(
      tmp_path,
      r'''
      const char* raw = R"({ not an initializer })";
      const char* normal = "{ still not an initializer }";

      struct Config {
         int first;
         int second;
      };

      Config config{
         .first = 1,
         .second = 2,
      };
      ''',
   )

   assert result.returncode == 0
   assert result.stderr == ""


def test_reports_missing_trailing_comma_in_returned_initializer(tmp_path: Path) -> None:
   result = run_check(
      tmp_path,
      """
      struct Config {
         int first;
         int second;
      };

      Config makeConfig() {
         return Config{
            .first = 1,
            .second = 2
         };
      }
      """,
   )

   assert result.returncode == 1
   assert "multi-line braced initializer should end with a trailing comma" in result.stderr


def test_reports_missing_path_explicitly(tmp_path: Path) -> None:
   result = subprocess.run(
      [sys.executable, str(CHECK_SCRIPT), str(tmp_path / "missing.cpp")],
      capture_output=True,
      text=True,
      check=False,
   )

   assert result.returncode == 2
   assert "path does not exist" in result.stderr
