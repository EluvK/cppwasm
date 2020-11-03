int a;
extern "C" void print(int a);
extern "C" void set(int input) {
    a = input;
}
extern "C" void add() {
    a = a + 1;
}
extern "C" void show() {
    print(a);
}