#include <support/component_impl.h>

class CDummyService2 : public sdv::CSdvObject
{
public:

    BEGIN_SDV_INTERFACE_MAP()
    END_SDV_INTERFACE_MAP()

    DECLARE_OBJECT_CLASS_TYPE(sdv::EObjectType::ComplexService)
    DECLARE_OBJECT_CLASS_NAME("DummyService #2")
};

DEFINE_SDV_OBJECT(CDummyService2)
