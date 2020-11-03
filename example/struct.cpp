class A {
public:
    int m_a{};
    double m_f{};
};
extern "C" void print(A a);
extern "C" void test(int input_i,double input_f) {
    A aa;
    aa.m_a = input_i;
    aa.m_f = input_f;
    print(aa);
}