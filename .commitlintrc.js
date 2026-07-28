import { execFileSync } from "node:child_process";
import { readdirSync } from "node:fs";
import { dirname, join } from "node:path";
import { fileURLToPath } from "node:url";
import { RuleConfigSeverity } from "@commitlint/types";

const repositoryRoot = dirname(fileURLToPath(import.meta.url));

// Only git-tracked directories are valid scopes, so that gitignored local
// directories (build/, node_modules/, ...) don't make lint results
// machine-dependent.
const trackedRootDirectories = execFileSync(
  "git",
  ["ls-tree", "-d", "--name-only", "HEAD"],
  { cwd: repositoryRoot, encoding: "utf8" },
)
  .split("\n")
  .filter((name) => name !== "" && !name.startsWith("."));

const siloSubdirectories = execFileSync(
  "git",
  ["ls-tree", "-d", "--name-only", "HEAD:src/silo"],
  { cwd: repositoryRoot, encoding: "utf8" },
)
  .split("\n")
  .filter((name) => name !== "" && !name.startsWith("."));

/**
 * @type {import('@commitlint/types').UserConfig}
 */
const Configuration = {
  extends: ["@commitlint/config-conventional"],
  rules: {
    "body-max-line-length": [RuleConfigSeverity.Disabled],
    "header-max-length": [RuleConfigSeverity.Error, "always", 200],
    "footer-max-line-length": [RuleConfigSeverity.Disabled],
    "scope-enum": [
      RuleConfigSeverity.Error,
      "always",
      [...trackedRootDirectories, ...siloSubdirectories, "silo", "main", "deps"],
    ],
  },
};

export default Configuration;
