"use client";

import { useParams } from "next/navigation";
import { useTranslations } from "next-intl";
import { Menu, BookOpen } from "lucide-react";
import { Link } from "@/i18n/navigation";
import { projectGroups } from "@/lib/projects";
import { ThemeToggle } from "@/components/theme-toggle";
import { LocaleToggle } from "@/components/locale-toggle";
import { Button } from "@/components/ui/button";
import { ScrollArea } from "@/components/ui/scroll-area";
import { Separator } from "@/components/ui/separator";
import {
  Sheet,
  SheetContent,
  SheetTrigger,
  SheetTitle,
} from "@/components/ui/sheet";
import { cn } from "@/lib/utils";

function SidebarContent() {
  const params = useParams();
  const currentProject = params.project as string | undefined;
  const t = useTranslations();

  return (
    <div className="flex h-full flex-col">
      <div className="p-4">
        <Link href="/" className="flex items-center gap-2 font-semibold">
          <BookOpen className="h-5 w-5" />
          <span className="text-sm">{t("common.appName")}</span>
        </Link>
      </div>
      <Separator />
      <ScrollArea className="flex-1 px-3 py-2">
        <div className="space-y-4">
          {projectGroups.map((group) => (
            <div key={group.key}>
              <h3 className="mb-1 px-2 text-xs font-medium tracking-wider text-muted-foreground uppercase">
                {t(`nav.${group.key}`)}
              </h3>
              <div className="space-y-0.5">
                {group.projects.map((project) => {
                  const Icon = project.icon;
                  const isActive = currentProject === project.id;
                  return (
                    <Link
                      key={project.id}
                      href={`/learn/${project.id}`}
                      className={cn(
                        "flex items-center gap-2 rounded-md px-2 py-1.5 text-sm transition-colors",
                        "hover:bg-accent hover:text-accent-foreground",
                        isActive && "bg-accent text-accent-foreground font-medium"
                      )}
                    >
                      <Icon className="h-4 w-4 shrink-0" />
                      <div className="min-w-0">
                        <div className="truncate">
                          {t(`projects.${project.id}.name`)}
                        </div>
                      </div>
                    </Link>
                  );
                })}
              </div>
            </div>
          ))}
        </div>
      </ScrollArea>
      <Separator />
      <div className="flex items-center justify-center gap-1 p-2">
        <ThemeToggle />
        <LocaleToggle />
      </div>
    </div>
  );
}

export function AppSidebar() {
  return (
    <>
      {/* Desktop sidebar */}
      <aside className="hidden w-60 shrink-0 border-r bg-sidebar text-sidebar-foreground md:block">
        <SidebarContent />
      </aside>

      {/* Mobile hamburger + sheet */}
      <div className="fixed top-3 left-3 z-50 md:hidden">
        <Sheet>
          <SheetTrigger asChild>
            <Button variant="outline" size="icon">
              <Menu className="h-4 w-4" />
            </Button>
          </SheetTrigger>
          <SheetContent side="left" className="w-60 p-0">
            <SheetTitle className="sr-only">Navigation</SheetTitle>
            <SidebarContent />
          </SheetContent>
        </Sheet>
      </div>
    </>
  );
}
