import type { DiagnosticRule } from "@/data/types";
import type { TestResult } from "@/lib/pyodide/test-parser";

/**
 * Match failed test results against diagnostic rules.
 * Returns a map of result index → matched DiagnosticRule.
 */
export function matchDiagnostics(
  results: TestResult[],
  rules?: DiagnosticRule[],
): Map<number, DiagnosticRule> {
  const map = new Map<number, DiagnosticRule>();
  if (!rules || rules.length === 0 || results.length === 0) return map;

  const allFailed = results.every((r) => !r.passed);

  // If all tests failed, look for _allFail rule
  if (allFailed) {
    const allFailRule = rules.find((r) => r.pattern === "_allFail");
    if (allFailRule) {
      map.set(0, allFailRule);
      return map;
    }
  }

  // For each failed test, find the first matching rule
  for (let i = 0; i < results.length; i++) {
    const r = results[i];
    if (r.passed) continue;

    for (const rule of rules) {
      if (rule.pattern === "_allFail") continue;
      if (matchesPattern(r.name, rule.pattern)) {
        map.set(i, rule);
        break;
      }
    }
  }

  return map;
}

function matchesPattern(
  testName: string,
  pattern: string | string[],
): boolean {
  if (Array.isArray(pattern)) {
    return pattern.some((p) => matchSingle(testName, p));
  }
  return matchSingle(testName, pattern);
}

function matchSingle(testName: string, pattern: string): boolean {
  // Wildcard suffix match: "test_negative*" matches "test_negative_number"
  if (pattern.endsWith("*")) {
    return testName.startsWith(pattern.slice(0, -1));
  }
  return testName === pattern;
}
