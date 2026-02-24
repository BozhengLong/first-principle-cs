import type { LearningModule } from "../types";
import { module00Introduction } from "./00-introduction";
import { module01Lexer } from "./01-lexer";

const modules: LearningModule[] = [module00Introduction, module01Lexer];

export function getModules(projectId: string): LearningModule[] {
  if (projectId === "tiny-interpreter") return modules;
  return [];
}

export function getModule(projectId: string, slug: string): LearningModule | undefined {
  return getModules(projectId).find((m) => m.slug === slug);
}

export function getFirstModule(projectId: string): LearningModule | undefined {
  const mods = getModules(projectId);
  return mods[0];
}

export function getAdjacentModules(projectId: string, slug: string) {
  const mods = getModules(projectId);
  const idx = mods.findIndex((m) => m.slug === slug);
  return {
    prev: idx > 0 ? mods[idx - 1] : undefined,
    next: idx < mods.length - 1 ? mods[idx + 1] : undefined,
  };
}
