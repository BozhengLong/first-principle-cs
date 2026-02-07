"""Main entry point for Tiny Interpreter."""

import sys
from .evaluator import Evaluator


def repl():
    """Run the Read-Eval-Print Loop."""
    evaluator = Evaluator()
    print("Tiny Interpreter v0.1.0")
    print("Type (exit) to quit")
    print()

    while True:
        try:
            source = input(">>> ")
            if source.strip() == "(exit)":
                break

            if not source.strip():
                continue

            result = evaluator.run(source)
            if result is not None:
                print(result)

        except KeyboardInterrupt:
            print("\nKeyboardInterrupt")
            break
        except Exception as e:
            print(f"Error: {e}")


def run_file(filename: str):
    """Run a file."""
    evaluator = Evaluator()

    try:
        with open(filename, 'r') as f:
            source = f.read()

        result = evaluator.run(source)
        if result is not None:
            print(result)

    except FileNotFoundError:
        print(f"Error: File not found: {filename}")
        sys.exit(1)
    except Exception as e:
        print(f"Error: {e}")
        sys.exit(1)


def main():
    """Main entry point."""
    if len(sys.argv) == 1:
        repl()
    elif len(sys.argv) == 2:
        run_file(sys.argv[1])
    else:
        print("Usage: python -m tiny_interpreter [file]")
        sys.exit(1)


if __name__ == "__main__":
    main()
