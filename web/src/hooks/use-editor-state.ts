"use client";

import { useCallback, useEffect, useState } from "react";

const STORAGE_PREFIX = "code:";

export function useEditorState(moduleId: string, skeleton?: string) {
  const [code, setCode] = useState(() => {
    if (typeof window === "undefined") return skeleton ?? "";
    const saved = localStorage.getItem(STORAGE_PREFIX + moduleId);
    return saved ?? skeleton ?? "";
  });

  // Auto-save debounced
  useEffect(() => {
    const timer = setTimeout(() => {
      if (typeof window !== "undefined") {
        localStorage.setItem(STORAGE_PREFIX + moduleId, code);
      }
    }, 1000);
    return () => clearTimeout(timer);
  }, [code, moduleId]);

  const reset = useCallback(() => {
    setCode(skeleton ?? "");
    if (typeof window !== "undefined") {
      localStorage.removeItem(STORAGE_PREFIX + moduleId);
    }
  }, [moduleId, skeleton]);

  return { code, setCode, reset };
}
