import { redirect } from "next/navigation";
import { setRequestLocale } from "next-intl/server";
import { notFound } from "next/navigation";
import { projects } from "@/lib/projects";
import { getFirstModule } from "@/data/tiny-interpreter";

const validProjectIds = projects.map((p) => p.id);

export function generateStaticParams() {
  return validProjectIds.map((project) => ({ project }));
}

export default async function LearnProjectPage({
  params,
}: {
  params: Promise<{ locale: string; project: string }>;
}) {
  const { locale, project } = await params;

  if (!validProjectIds.includes(project)) {
    notFound();
  }

  setRequestLocale(locale);

  const firstModule = getFirstModule(project);
  if (firstModule) {
    redirect(`/${locale}/learn/${project}/${firstModule.slug}`);
  }

  notFound();
}
