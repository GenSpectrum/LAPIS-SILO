import { RuleConfigSeverity } from "@commitlint/types";

/**
 * @type {import('@commitlint/types').UserConfig}
 */
const Configuration = {
  extends: ["@commitlint/config-conventional"],
  rules: {
    "body-max-line-length": [RuleConfigSeverity.Error, "always", 200],
    "header-max-length": [RuleConfigSeverity.Error, "always", 200],
    "footer-max-line-length": [RuleConfigSeverity.Disabled],
  },
};

export default Configuration;
