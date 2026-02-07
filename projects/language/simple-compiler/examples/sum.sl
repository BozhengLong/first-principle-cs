// Sum of numbers from 1 to N
int main() {
    int n = 10;
    int sum = 0;
    int i = 1;

    while (i <= n) {
        sum = sum + i;
        i = i + 1;
    }

    return sum;  // Returns 55 (1+2+...+10)
}
