import { RuleConfigSeverity } from "@commitlint/types";

/**
 * @type {import('@commitlint/types').UserConfig}
 */
const Configuration = {
  extends: ["@commitlint/config-conventional"],
  rules: {
    "body-max-line-length": [RuleConfigSeverity.Error, "always", 200],
  },
};

export default Configuration;
