#!/usr/bin/env python3
"""Check trailing commas in multi-line C++ braced initializers.

This helper intentionally uses a lightweight tokenizer plus a few statement-level heuristics instead
of full C++ parsing. It is meant to cover the initializer forms used in this repository while
avoiding normal blocks such as class, namespace, and function bodies.
"""

from __future__ import annotations

import argparse
from dataclasses import dataclass, replace
from pathlib import Path
import re
import subprocess
import sys

CPP_EXTENSIONS = {".cpp", ".h", ".hpp"}
DEFAULT_PATHS = ("src", "app/src", "wasm/src")
BLOCK_KEYWORDS = {"if", "for", "while", "switch", "catch", "else", "do", "try"}
TYPE_KEYWORDS = {"class", "struct", "union", "enum", "namespace"}
INITIALIZER_CLOSE_FOLLOWERS = {";", ",", ")", "]"}
EXPRESSION_CUES = {"=", "return", "co_return", ",", "(", "[", "{", "?", "throw"}
STATEMENT_BOUNDARIES = {";", "{", "}"}


@dataclass(frozen=True)
class Token:
   text: str
   line: int
   column: int
   index: int
   pair_index: int | None = None


def find_cpp_files(paths: list[str]) -> list[Path]:
   """Expand file and directory arguments into the C++ files that should be checked."""
   collected: list[Path] = []
   seen: set[Path] = set()
   for raw_path in paths:
      path = Path(raw_path)
      if not path.exists():
         raise FileNotFoundError(f"path does not exist: {path}")
      if path.is_file():
         if path.suffix not in CPP_EXTENSIONS:
            raise ValueError(f"unsupported file extension: {path}")
         resolved = path.resolve()
         if resolved not in seen:
            seen.add(resolved)
            collected.append(path)
         continue
      for candidate in path.rglob("*"):
         if candidate.is_file() and candidate.suffix in CPP_EXTENSIONS:
            resolved = candidate.resolve()
            if resolved not in seen:
               seen.add(resolved)
               collected.append(candidate)
   return sorted(collected)


def parse_changed_lines(diff_output: str) -> set[int]:
   changed_lines: set[int] = set()
   for line in diff_output.splitlines():
      match = re.match(r"@@ -\d+(?:,\d+)? \+(\d+)(?:,(\d+))? @@", line)
      if match is None:
         continue
      start = int(match.group(1))
      count = int(match.group(2) or "1")
      for changed_line in range(start, start + count):
         changed_lines.add(changed_line)
   return changed_lines


def tokenize(source: str) -> list[Token]:
   """Tokenize source while skipping comments and string literals."""
   tokens: list[Token] = []
   brace_stack: list[int] = []
   paren_stack: list[int] = []
   bracket_stack: list[int] = []
   i = 0
   line = 1
   column = 1
   length = len(source)

   def advance(count: int = 1) -> None:
      nonlocal i, line, column
      for _ in range(min(count, length - i)):
         if source[i] == "\n":
            line += 1
            column = 1
         else:
            column += 1
         i += 1

   def add_token(text: str, start_line: int, start_column: int, start_index: int) -> None:
      tokens.append(Token(text, start_line, start_column, start_index))

   while i < length:
      current = source[i]
      if current.isspace():
         advance()
         continue
      if source.startswith("//", i):
         while i < length and source[i] != "\n":
            advance()
         continue
      if source.startswith("/*", i):
         advance(2)
         while i < length and not source.startswith("*/", i):
            advance()
         if i < length:
            advance(2)
         continue
      if source.startswith("R\"", i):
         raw_start_line, raw_start_column, raw_start_index = line, column, i
         delimiter_end = source.find("(", i + 2)
         if delimiter_end == -1:
            add_token("R", raw_start_line, raw_start_column, raw_start_index)
            advance()
            continue
         raw_delimiter = source[i + 2 : delimiter_end]
         closing = f"){raw_delimiter}\""
         advance(delimiter_end - i + 1)
         while i < length and not source.startswith(closing, i):
            advance()
         if i < length:
            advance(len(closing))
         continue
      if current in {'"', "'"}:
         delimiter = current
         advance()
         while i < length:
            if source[i] == "\\":
               advance(2)
               continue
            if source[i] == delimiter:
               advance()
               break
            advance()
         continue
      start_line, start_column, start_index = line, column, i
      three_char = source[i : i + 3]
      two_char = source[i : i + 2]
      if three_char in {"<=>", "...", "<<=", ">>="}:
         add_token(three_char, start_line, start_column, start_index)
         advance(3)
         continue
      if two_char in {
         "::",
         "->",
         "++",
         "--",
         "==",
         "!=",
         "<=",
         ">=",
         "&&",
         "||",
         "+=",
         "-=",
         "*=",
         "/=",
         "%=",
         "&=",
         "|=",
         "^=",
         "<<",
         ">>",
         "<<=",
         ">>=",
      }:
         add_token(two_char, start_line, start_column, start_index)
         advance(2)
         continue
      if current.isalpha() or current == "_":
         end = i + 1
         while end < length and (source[end].isalnum() or source[end] == "_"):
            end += 1
         text = source[i:end]
         add_token(text, start_line, start_column, start_index)
         advance(end - i)
         continue
      if current.isdigit():
         end = i + 1
         while end < length and (source[end].isalnum() or source[end] in "._'"):
            end += 1
         add_token(source[i:end], start_line, start_column, start_index)
         advance(end - i)
         continue
      add_token(current, start_line, start_column, start_index)
      advance()

   updated_tokens = list(tokens)
   for index, token in enumerate(tokens):
      if token.text == "{":
         brace_stack.append(index)
      elif token.text == "}":
         if brace_stack:
            open_index = brace_stack.pop()
            updated_tokens[open_index] = replace(updated_tokens[open_index], pair_index=index)
            updated_tokens[index] = replace(updated_tokens[index], pair_index=open_index)
      elif token.text == "(":
         paren_stack.append(index)
      elif token.text == ")":
         if paren_stack:
            open_index = paren_stack.pop()
            updated_tokens[open_index] = replace(updated_tokens[open_index], pair_index=index)
            updated_tokens[index] = replace(updated_tokens[index], pair_index=open_index)
      elif token.text == "[":
         bracket_stack.append(index)
      elif token.text == "]":
         if bracket_stack:
            open_index = bracket_stack.pop()
            updated_tokens[open_index] = replace(updated_tokens[open_index], pair_index=index)
            updated_tokens[index] = replace(updated_tokens[index], pair_index=open_index)
   return updated_tokens


def previous_token(tokens: list[Token], index: int) -> Token | None:
   return tokens[index - 1] if index > 0 else None


def next_token(tokens: list[Token], index: int) -> Token | None:
   return tokens[index + 1] if index + 1 < len(tokens) else None


def statement_start_index(tokens: list[Token], open_index: int) -> int:
   index = open_index - 1
   while index >= 0 and tokens[index].text not in STATEMENT_BOUNDARIES:
      index -= 1
   return index + 1


def is_statement_expression_context(tokens: list[Token], call_head_index: int) -> bool:
   index = call_head_index - 1
   while index >= 0 and tokens[index].text not in STATEMENT_BOUNDARIES:
      if tokens[index].text in EXPRESSION_CUES:
         return True
      index -= 1
   return False


def is_initializer_open(tokens: list[Token], open_index: int, close_index: int) -> bool:
   """Best-effort detection of braced initializer contexts that should require a trailing comma."""
   open_token = tokens[open_index]
   close_token = tokens[close_index]
   if open_token.line == close_token.line:
      return False

   before_open = previous_token(tokens, open_index)
   after_close = next_token(tokens, close_index)
   if before_open is None or after_close is None:
      return False
   if after_close.text not in INITIALIZER_CLOSE_FOLLOWERS:
      return False

   statement_tokens = tokens[statement_start_index(tokens, open_index) : open_index]
   statement_token_texts = {token.text for token in statement_tokens}
   if statement_token_texts.intersection(TYPE_KEYWORDS):
      return False
   if ")" in statement_token_texts and not statement_token_texts.intersection(EXPRESSION_CUES):
      return False

   if before_open.text in BLOCK_KEYWORDS or before_open.text == "]":
      return False
   if before_open.text in TYPE_KEYWORDS:
      return False

   before_before_open = tokens[open_index - 2] if open_index >= 2 else None
   if before_before_open is not None:
      if before_before_open.text in TYPE_KEYWORDS and before_open.text.isidentifier():
         return False

   if before_open.text == ")":
      if before_open.pair_index is None:
         return False
      call_open = tokens[before_open.pair_index]
      call_head = previous_token(tokens, before_open.pair_index)
      if call_head is None:
         return False
      if call_head.text in BLOCK_KEYWORDS:
         return False
      if call_head.text == "]":
         return False
      return is_statement_expression_context(tokens, before_open.pair_index)

   return True


def find_violations(source: str) -> list[tuple[int, int, int, int]]:
   tokens = tokenize(source)
   violations: list[tuple[int, int, int, int]] = []
   for index, token in enumerate(tokens):
      if token.text != "{" or token.pair_index is None:
         continue
      close_index = token.pair_index
      if not is_initializer_open(tokens, index, close_index):
         continue
      last_inner_token = previous_token(tokens, close_index)
      if last_inner_token is None or last_inner_token.text == ",":
         continue
      violations.append((token.line, tokens[close_index].line, last_inner_token.line, last_inner_token.column))
   return violations


def check_file(path: Path) -> list[str]:
   return check_file_lines(path, None)


def changed_lines_for_file(diff_base: str, path: Path) -> set[int]:
   if has_staged_changes(path):
      return staged_changed_lines_for_file(diff_base, path)
   return working_tree_changed_lines_for_file(diff_base, path)


def working_tree_changed_lines_for_file(diff_base: str, path: Path) -> set[int]:
   result = subprocess.run(
      ["git", "diff", "--unified=0", "--no-color", diff_base, "--", str(path)],
      capture_output=True,
      text=True,
      check=True,
   )
   return parse_changed_lines(result.stdout)


def staged_changed_lines_for_file(diff_base: str, path: Path) -> set[int]:
   result = subprocess.run(
      ["git", "diff", "--cached", "--unified=0", "--no-color", diff_base, "--", str(path)],
      capture_output=True,
      text=True,
      check=True,
   )
   return parse_changed_lines(result.stdout)


def has_staged_changes(path: Path) -> bool:
   result = subprocess.run(
      ["git", "diff", "--cached", "--name-only", "--", str(path)],
      capture_output=True,
      text=True,
      check=True,
   )
   return bool(result.stdout.strip())


def read_staged_file(path: Path) -> str:
   result = subprocess.run(
      ["git", "show", f":{path.as_posix()}"],
      capture_output=True,
      check=True,
   )
   return result.stdout.decode("utf-8")


def check_file_lines(
   path: Path,
   changed_lines: set[int] | None,
   source_override: str | None = None,
) -> list[str]:
   try:
      source = source_override if source_override is not None else path.read_text(encoding="utf-8")
   except UnicodeDecodeError:
      return [f"{path}:1:1: could not decode file as UTF-8"]
   messages = []
   for start_line, end_line, line, column in find_violations(source):
      if changed_lines is not None and changed_lines.isdisjoint(range(start_line, end_line + 1)):
         continue
      messages.append(
         f"{path}:{line}:{column}: multi-line braced initializer should end with a trailing comma"
      )
   return messages


def main() -> int:
   parser = argparse.ArgumentParser(
      description="Check that multi-line braced initializers end with a trailing comma.",
   )
   parser.add_argument(
      "--diff-base",
      help="Only report violations in lines changed relative to the given git revision.",
   )
   parser.add_argument("paths", nargs="*", default=list(DEFAULT_PATHS))
   args = parser.parse_args()

   try:
      files = find_cpp_files(args.paths)
   except (FileNotFoundError, ValueError, subprocess.CalledProcessError) as error:
      if isinstance(error, subprocess.CalledProcessError) and error.stderr:
         print(error.stderr.strip(), file=sys.stderr)
      else:
         print(error, file=sys.stderr)
      return 2
   messages: list[str] = []
   for file_path in files:
      changed_lines = None
      source_override = None
      if args.diff_base is not None:
         changed_lines = changed_lines_for_file(args.diff_base, file_path)
         if not changed_lines:
            continue
         if has_staged_changes(file_path):
            source_override = read_staged_file(file_path)
      messages.extend(check_file_lines(file_path, changed_lines, source_override))
   if messages:
      print("\n".join(messages), file=sys.stderr)
      return 1
   return 0


if __name__ == "__main__":
   raise SystemExit(main())
