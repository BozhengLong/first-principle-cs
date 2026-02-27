# Enhanced Visualization Testing Checklist

## Pre-Testing Setup

- [ ] Dependencies installed: `npm install` in `web/` directory
- [ ] Build successful: `npm run build` completes without errors
- [ ] Dev server running: `npm run dev` on port 3000
- [ ] Browser opened to: `http://localhost:3000`

## Test Suite 1: Basic Step-Through (Module 04: Evaluator-Basic)

### Setup
1. Navigate to: `/zh/learn/tiny-interpreter/evaluator-basic`
2. Click "Visualization" tab
3. Enter expression: `(+ 1 (* 2 3))`
4. Click "Visualize" button

### Expected Results
- [ ] Step controls appear at top
- [ ] Shows "Step 1 / N" (where N > 1)
- [ ] AST tree displays with nodes
- [ ] Environment chain shows global frame
- [ ] No error messages

### Step Navigation Tests
- [ ] Click "Next" (⏩) - step counter increments
- [ ] Click "Next" repeatedly - reaches final step
- [ ] AST highlights move through tree
- [ ] Current node has thick border + glow
- [ ] Executed nodes at full opacity
- [ ] Not-yet-executed nodes at 40% opacity
- [ ] Click "Previous" (⏪) - step counter decrements
- [ ] Click "First" (⏮️) - jumps to step 1
- [ ] Click "Last" (⏭️) - jumps to final step
- [ ] Drag slider - jumps to selected step

### Playback Tests
- [ ] Click "Play" (▶️) - auto-advances
- [ ] Steps advance automatically
- [ ] Click "Pause" (⏸️) - stops advancing
- [ ] Click "Slow" - playback slows down
- [ ] Click "Fast" - playback speeds up
- [ ] Playback stops at final step

### Visual Feedback Tests
- [ ] Step message updates (e.g., "Evaluating Number")
- [ ] AST node hover shows scale effect
- [ ] Current node has visible glow
- [ ] Smooth transitions between steps

## Test Suite 2: Environment Evolution (Module 03: Environment)

### Setup
1. Navigate to: `/zh/learn/tiny-interpreter/environment`
2. Click "Visualization" tab
3. Enter expression: `(define x 10)`
4. Click "Visualize"

### Expected Results
- [ ] Step controls appear
- [ ] Environment chain shows global frame
- [ ] Step through execution
- [ ] New binding "x: 10" appears in environment
- [ ] Binding appears with animation (if implemented)

### Variable Lookup Test
1. Enter: `(define y (+ x 5))`
2. Click "Visualize"
3. Step through

- [ ] Shows lookup of "x" variable
- [ ] Shows evaluation of "(+ x 5)"
- [ ] Shows definition of "y: 15"
- [ ] Environment shows both x and y bindings

## Test Suite 3: Closure Visualization (Module 05: Evaluator-Lambda)

### Setup
1. Navigate to: `/zh/learn/tiny-interpreter/evaluator-lambda`
2. Click "Visualization" tab
3. Enter expression: `(define add5 (lambda (x) (+ x 5)))`
4. Click "Visualize"

### Expected Results
- [ ] Step controls appear
- [ ] Shows lambda creation step
- [ ] Step message mentions "closure_create"
- [ ] Closure visualization section appears (if implemented)
- [ ] Shows closure structure (params, body, captured env)

### Closure Application Test
1. Enter: `(define add5 (lambda (x) (+ x 5)))\n(add5 10)`
2. Click "Visualize"
3. Step through

- [ ] Shows closure creation
- [ ] Shows closure application
- [ ] Shows new environment frame for closure call
- [ ] Shows parameter binding (x: 10)
- [ ] Shows body evaluation
- [ ] Shows return value (15)

## Test Suite 4: Complex Expressions

### Nested Arithmetic
Expression: `(+ (+ 1 2) (* 3 4))`

- [ ] Visualizes successfully
- [ ] Shows evaluation of inner expressions first
- [ ] Shows outer addition last
- [ ] Correct step count (should be ~7-9 steps)

### Conditional Branching
Expression: `(if #t (+ 1 2) (* 3 4))`

- [ ] Visualizes successfully
- [ ] Shows condition evaluation
- [ ] Shows only true branch evaluation
- [ ] Does NOT show false branch evaluation
- [ ] Correct final result (3)

### Multiple Definitions
Expression: `(define x 5)\n(define y 10)\n(+ x y)`

- [ ] Visualizes successfully
- [ ] Shows first definition
- [ ] Shows second definition
- [ ] Shows final addition
- [ ] Environment shows both bindings

## Test Suite 5: Edge Cases

### Empty Expression
Expression: `` (empty)

- [ ] Shows appropriate error or empty state
- [ ] No crash
- [ ] Clear error message

### Syntax Error
Expression: `(+ 1 2`

- [ ] Shows error message
- [ ] Error is clearly displayed
- [ ] No step controls appear
- [ ] Can recover by entering valid expression

### Undefined Variable
Expression: `(+ x 1)`

- [ ] Shows error during execution
- [ ] Error message mentions undefined variable
- [ ] Step controls may appear but execution fails

### Very Long Trace
Expression: `(+ 1 (+ 2 (+ 3 (+ 4 (+ 5 6)))))`

- [ ] Visualizes successfully
- [ ] Many steps (20+)
- [ ] Slider works smoothly
- [ ] No performance issues
- [ ] Can navigate to any step

## Test Suite 6: UI/UX Tests

### Responsive Design
- [ ] Works on desktop (1920x1080)
- [ ] Works on laptop (1366x768)
- [ ] Works on tablet (768x1024)
- [ ] Controls remain accessible
- [ ] Visualizations scale appropriately

### Theme Switching
- [ ] Works in light mode
- [ ] Works in dark mode
- [ ] Colors remain readable
- [ ] Highlights visible in both themes

### Browser Compatibility
- [ ] Chrome/Edge (latest)
- [ ] Firefox (latest)
- [ ] Safari (latest)
- [ ] No console errors

### Performance
- [ ] Initial load < 3 seconds
- [ ] Visualization generation < 2 seconds
- [ ] Step navigation < 100ms
- [ ] Smooth animations (60fps)
- [ ] No memory leaks (check DevTools)

## Test Suite 7: Integration Tests

### Module Switching
1. Start on Module 04
2. Visualize an expression
3. Navigate to Module 05
4. Visualize a different expression

- [ ] State resets correctly
- [ ] No stale data from previous module
- [ ] Step controls reset to step 1

### Input Changes
1. Visualize expression A
2. Change input to expression B
3. Click "Visualize" again

- [ ] Old visualization clears
- [ ] New visualization loads
- [ ] Step counter resets
- [ ] No mixing of old/new data

### Rapid Clicking
1. Click "Next" rapidly 10 times
2. Click "Previous" rapidly 10 times
3. Drag slider back and forth

- [ ] No crashes
- [ ] State remains consistent
- [ ] UI remains responsive

## Bug Report Template

If you find issues, report with:

```
**Module**: [e.g., evaluator-basic]
**Expression**: [e.g., (+ 1 2)]
**Steps to Reproduce**:
1. ...
2. ...

**Expected**: [what should happen]
**Actual**: [what actually happened]
**Console Errors**: [paste any errors]
**Screenshot**: [if applicable]
```

## Success Criteria

✅ All basic step-through tests pass
✅ Environment evolution visible
✅ Closure visualization works (if implemented)
✅ No crashes on edge cases
✅ Performance acceptable (< 2s per visualization)
✅ Works in multiple browsers
✅ Responsive on different screen sizes

## Known Limitations (Expected)

- Closure data extraction may be incomplete
- Lookup path animation may not be implemented
- Specific binding highlights may not work yet
- Node click handlers may not be wired up

These are documented in IMPLEMENTATION_SUMMARY.md and are acceptable for initial release.
