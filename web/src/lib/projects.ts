import {
  Terminal,
  Cpu,
  Monitor,
  HardDrive,
  Database,
  GitBranch,
  Network,
  Globe,
  type LucideIcon,
} from "lucide-react";

export type ProjectGroup = "language" | "systems" | "data" | "distributed";

export interface Project {
  id: string;
  group: ProjectGroup;
  icon: LucideIcon;
}

export const projects: Project[] = [
  { id: "tiny-interpreter", group: "language", icon: Terminal },
  { id: "simple-compiler", group: "language", icon: Cpu },
  { id: "mini-os", group: "systems", icon: Monitor },
  { id: "simple-fs", group: "systems", icon: HardDrive },
  { id: "storage-engine", group: "data", icon: Database },
  { id: "tx-manager", group: "data", icon: GitBranch },
  { id: "consensus", group: "distributed", icon: Network },
  { id: "dist-kv", group: "distributed", icon: Globe },
];

export const projectGroups: { key: ProjectGroup; projects: Project[] }[] = [
  {
    key: "language",
    projects: projects.filter((p) => p.group === "language"),
  },
  {
    key: "systems",
    projects: projects.filter((p) => p.group === "systems"),
  },
  {
    key: "data",
    projects: projects.filter((p) => p.group === "data"),
  },
  {
    key: "distributed",
    projects: projects.filter((p) => p.group === "distributed"),
  },
];
