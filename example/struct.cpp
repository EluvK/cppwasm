class A {
public:
    int m_a{0};
    int m_b{0};
};
extern "C" void print(A a);
extern "C" void test(int input_a,int input_b) {
    A aa;
    aa.m_a = input_a;
    aa.m_b = input_b;
    print(aa);
}