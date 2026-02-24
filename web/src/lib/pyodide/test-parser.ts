export interface TestResult {
  name: string;
  className: string;
  passed: boolean;
  message?: string;
}

export function parseTestOutput(output: string): TestResult[] {
  const results: TestResult[] = [];
  const lines = output.split("\n");

  for (const line of lines) {
    // Match pytest verbose output: "test_skeleton.py::TestBasics::test_empty_input PASSED"
    const match = line.match(
      /::(\w+)::(\w+)\s+(PASSED|FAILED|ERROR)/
    );
    if (match) {
      const [, className, testName, status] = match;
      const result: TestResult = {
        name: testName,
        className,
        passed: status === "PASSED",
      };
      if (status !== "PASSED") {
        // Try to find failure message in subsequent lines
        const idx = lines.indexOf(line);
        const failLines: string[] = [];
        for (let i = idx + 1; i < lines.length && i < idx + 10; i++) {
          if (lines[i].match(/::(\w+)::(\w+)\s+(PASSED|FAILED|ERROR)/)) break;
          if (lines[i].startsWith("FAILED") || lines[i].startsWith("=")) break;
          if (lines[i].trim()) failLines.push(lines[i]);
        }
        if (failLines.length > 0) {
          result.message = failLines.join("\n");
        }
      }
      results.push(result);
    }
  }

  return results;
}
