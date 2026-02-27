"use client";

import { useEffect } from "react";
import {
  ChevronsLeft,
  ChevronLeft,
  Play,
  Pause,
  ChevronRight,
  ChevronsRight,
} from "lucide-react";
import { Button } from "@/components/ui/button";
import { Slider } from "@/components/ui/slider";
import { useWorkspace } from "@/contexts/workspace-context";

interface StepControlsProps {
  totalSteps: number;
  currentMessage?: string;
}

export function StepControls({ totalSteps, currentMessage }: StepControlsProps) {
  const {
    currentStep,
    setCurrentStep,
    isPlaying,
    setIsPlaying,
    playbackSpeed,
    setPlaybackSpeed,
  } = useWorkspace();

  // Auto-advance when playing
  useEffect(() => {
    if (!isPlaying) return;
    if (currentStep >= totalSteps - 1) {
      setIsPlaying(false);
      return;
    }

    const timer = setTimeout(() => {
      setCurrentStep(currentStep + 1);
    }, playbackSpeed);

    return () => clearTimeout(timer);
  }, [isPlaying, currentStep, totalSteps, playbackSpeed, setCurrentStep, setIsPlaying]);

  const handleFirst = () => {
    setCurrentStep(0);
    setIsPlaying(false);
  };

  const handlePrev = () => {
    setCurrentStep(Math.max(0, currentStep - 1));
    setIsPlaying(false);
  };

  const handlePlayPause = () => {
    if (currentStep >= totalSteps - 1) {
      setCurrentStep(0);
    }
    setIsPlaying(!isPlaying);
  };

  const handleNext = () => {
    setCurrentStep(Math.min(totalSteps - 1, currentStep + 1));
    setIsPlaying(false);
  };

  const handleLast = () => {
    setCurrentStep(totalSteps - 1);
    setIsPlaying(false);
  };

  const handleSliderChange = (values: number[]) => {
    setCurrentStep(values[0]);
    setIsPlaying(false);
  };

  const speedOptions = [
    { label: "Slow", value: 1500 },
    { label: "Medium", value: 1000 },
    { label: "Fast", value: 500 },
  ];

  return (
    <div className="space-y-3 rounded-lg border bg-card p-4">
      {/* Step info */}
      <div className="flex items-center justify-between">
        <div className="text-sm font-medium">
          Step {currentStep + 1} / {totalSteps}
        </div>
        {currentMessage && (
          <div className="text-xs text-muted-foreground">{currentMessage}</div>
        )}
      </div>

      {/* Progress slider */}
      <Slider
        value={[currentStep]}
        onValueChange={handleSliderChange}
        max={totalSteps - 1}
        step={1}
        className="w-full"
      />

      {/* Playback controls */}
      <div className="flex items-center justify-between">
        <div className="flex items-center gap-1">
          <Button
            variant="ghost"
            size="icon"
            onClick={handleFirst}
            disabled={currentStep === 0}
            className="h-8 w-8"
          >
            <ChevronsLeft className="h-4 w-4" />
          </Button>
          <Button
            variant="ghost"
            size="icon"
            onClick={handlePrev}
            disabled={currentStep === 0}
            className="h-8 w-8"
          >
            <ChevronLeft className="h-4 w-4" />
          </Button>
          <Button
            variant="ghost"
            size="icon"
            onClick={handlePlayPause}
            className="h-8 w-8"
          >
            {isPlaying ? (
              <Pause className="h-4 w-4" />
            ) : (
              <Play className="h-4 w-4" />
            )}
          </Button>
          <Button
            variant="ghost"
            size="icon"
            onClick={handleNext}
            disabled={currentStep >= totalSteps - 1}
            className="h-8 w-8"
          >
            <ChevronRight className="h-4 w-4" />
          </Button>
          <Button
            variant="ghost"
            size="icon"
            onClick={handleLast}
            disabled={currentStep >= totalSteps - 1}
            className="h-8 w-8"
          >
            <ChevronsRight className="h-4 w-4" />
          </Button>
        </div>

        {/* Speed control */}
        <div className="flex items-center gap-2">
          <span className="text-xs text-muted-foreground">Speed:</span>
          {speedOptions.map((option) => (
            <Button
              key={option.value}
              variant={playbackSpeed === option.value ? "default" : "ghost"}
              size="sm"
              onClick={() => setPlaybackSpeed(option.value)}
              className="h-7 px-2 text-xs"
            >
              {option.label}
            </Button>
          ))}
        </div>
      </div>
    </div>
  );
}
