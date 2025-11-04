#include <support/component_impl.h>

class CModuleTestComponent1 : public sdv::CSdvObject
{
public:

    BEGIN_SDV_INTERFACE_MAP()
    END_SDV_INTERFACE_MAP()

    DECLARE_OBJECT_CLASS_TYPE(sdv::EObjectType::Utility)
    DECLARE_OBJECT_CLASS_NAME("ModuleTestComponent1")
private:
};
DEFINE_SDV_OBJECT(CModuleTestComponent1)

class CModuleTestComponent2 : public sdv::CSdvObject
{
public:

    BEGIN_SDV_INTERFACE_MAP()
    END_SDV_INTERFACE_MAP()

    DECLARE_OBJECT_CLASS_TYPE(sdv::EObjectType::Utility)
    DECLARE_OBJECT_CLASS_NAME("ModuleTestComponent2")
private:
};
DEFINE_SDV_OBJECT(CModuleTestComponent2)

