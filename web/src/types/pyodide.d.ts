declare interface PyodideInterface {
  runPythonAsync(code: string): Promise<unknown>;
  loadPackage(packages: string | string[]): Promise<void>;
  FS: {
    writeFile(path: string, data: string): void;
    readFile(path: string, opts?: { encoding: string }): string;
    unlink(path: string): void;
    mkdir(path: string): void;
  };
  globals: {
    get(name: string): unknown;
    set(name: string, value: unknown): void;
  };
}

declare function loadPyodide(config?: {
  indexURL?: string;
}): Promise<PyodideInterface>;

interface Window {
  loadPyodide: typeof loadPyodide;
}
