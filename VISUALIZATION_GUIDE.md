# Enhanced Visualization Interactivity - Quick Start Guide

## What's New

The tiny-interpreter learning path now features **interactive, step-through execution visualization**. Instead of seeing just the final state, learners can now:

1. **Step through code execution** - See each evaluation step one at a time
2. **Watch the AST come alive** - Current node highlights, executed nodes brighten
3. **See environment evolution** - Watch bindings appear with animations
4. **Understand closures visually** - See what environment a closure captures

## How to Use

### 1. Start the Development Server

```bash
cd web
npm run dev
```

### 2. Navigate to a Module

Open your browser to `http://localhost:3000/zh/learn/tiny-interpreter/evaluator-basic`

### 3. Try the Interactive Visualization

1. Click the **"Visualization"** tab
2. Enter an expression (default: `(+ 1 (* 2 3))`)
3. Click **"Visualize"**
4. You'll see:
   - **Step Controls** at the top (⏮️ ⏪ ▶️ ⏩ ⏭️)
   - **Current step** indicator (e.g., "Step 3 / 15")
   - **Step message** (e.g., "Evaluating Number")
   - **AST tree** with current node highlighted
   - **Environment chain** showing current state

### 4. Step Through Execution

- **Next (⏩)**: Advance one step
- **Previous (⏪)**: Go back one step
- **Play (▶️)**: Auto-advance through all steps
- **Pause (⏸️)**: Stop auto-advance
- **First (⏮️)**: Jump to beginning
- **Last (⏭️)**: Jump to end
- **Slider**: Drag to any step

### 5. Adjust Playback Speed

Click **Slow**, **Medium**, or **Fast** to change auto-advance speed.

## Example Expressions to Try

### Module 04: Evaluator-Basic
```scheme
(+ 1 (* 2 3))          ; See arithmetic evaluation
(if #t 10 20)          ; See conditional branching
(+ (+ 1 2) (* 3 4))    ; See nested expressions
```

### Module 03: Environment
```scheme
(define x 10)          ; Watch binding creation
(define y (+ x 5))     ; See variable lookup
```

### Module 05: Evaluator-Lambda
```scheme
(define add5 (lambda (x) (+ x 5)))  ; See closure creation
(add5 10)                            ; See closure application
```

## Visual Cues

### AST Tree
- **Thick border + glow**: Current node being evaluated
- **Full opacity**: Already executed
- **40% opacity**: Not yet executed
- **Hover**: Scale up slightly

### Environment Chain
- **Green flash**: New binding just created
- **Pulsing border**: Frame involved in lookup
- **Animated arrow**: Parent chain connection

### Closure Visualization
- **Green box**: Captured environment
- **Arrow**: Points from closure to captured env
- **Step number**: When closure was created

## Troubleshooting

### No Step Controls Appear
- Make sure you're on a module with `vizType: "evaluator"` or `"closure"`
- Verify the expression executed successfully (no error message)
- Check that trace data was generated (should see step counter)

### Visualization Doesn't Update
- Click "Visualize" button after changing the input
- Make sure Pyodide has loaded (wait a few seconds on first load)

### Performance Issues
- Very long traces (100+ steps) may be slow
- Try simpler expressions first
- Use speed control to slow down playback

## Architecture Overview

```
User Input → Pyodide (Python) → TracingEvaluator
    ↓
Execution Trace (JSON)
    ↓
React Components:
  - StepControls (playback UI)
  - ASTTree (interactive tree)
  - EnvChain (animated frames)
  - ClosureViz (closure details)
```

## Next Steps

After trying the basic features:

1. **Experiment with complex expressions** - See how nested calls work
2. **Try error cases** - What happens with undefined variables?
3. **Explore closures** - How does a closure "remember" its environment?
4. **Compare different approaches** - Try solving the same problem different ways

## Feedback

If you encounter issues or have suggestions:
1. Check the browser console for errors
2. Verify all dependencies are installed (`npm install`)
3. Try rebuilding (`npm run build`)
4. Report issues with specific expressions that fail

---

**Happy exploring! 🚀**
