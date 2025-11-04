#include <support/component_impl.h>

/**
 * @brief Dummy service class with loads of extra settings-
 */
class CDummyService1 : public sdv::CSdvObject
{
public:

    BEGIN_SDV_INTERFACE_MAP()
    END_SDV_INTERFACE_MAP()

    DECLARE_OBJECT_CLASS_TYPE(sdv::EObjectType::ComplexService)
    DECLARE_OBJECT_CLASS_NAME("DummyService #1")
    DECLARE_OBJECT_CLASS_ALIAS("Dummy1", "DummySvc1")
    DECLARE_DEFAULT_OBJECT_NAME("MyDummy")
    DECLARE_OBJECT_SINGLETON()
    DECLARE_OBJECT_DEPENDENCIES("DummyDevice", "DummyService #2")
};
DEFINE_SDV_OBJECT(CDummyService1)

/**
 * @brief Dummy device class.
 */
class CDummyDevice : public sdv::CSdvObject
{
public:

    BEGIN_SDV_INTERFACE_MAP()
    END_SDV_INTERFACE_MAP()

    DECLARE_OBJECT_CLASS_TYPE(sdv::EObjectType::Device)
    DECLARE_OBJECT_CLASS_NAME("DummyDevice")
};
DEFINE_SDV_OBJECT(CDummyDevice)
