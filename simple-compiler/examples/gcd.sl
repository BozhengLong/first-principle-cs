// GCD (Greatest Common Divisor) using Euclidean algorithm
int main() {
    int a = 48;
    int b = 18;

    while (b != 0) {
        int temp = b;
        b = a % b;
        a = temp;
    }

    return a;  // Returns 6
}
