#include "net.hpp"

int main() {
    using namespace LPNN;
    Net<double>::Topology top{
        {2, Layer<double>::ActivationFunction::TANH}
    };
    Net<double> net{top};
})