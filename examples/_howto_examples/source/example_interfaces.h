#include <interfaces/core.h>
#include <support/signal_support.h>

interface ISayHello                  ///< interface example ISayHello 
{
	virtual void SayHello() = 0;     ///< function og the interface
	static constexpr ::sdv::interface_id _id = 0xA012345678900100; ///< Interface Id
};

interface ISayGoodbye                ///< interface example ISayGoodbye 
{
	virtual void SayGoodbye() = 0;   ///< function og the interface
	static constexpr ::sdv::interface_id _id = 0xA012345678900200; ///< Interface Id
};

interface IShowExample               ///< interface example IShowExample
{
	virtual void Show() = 0;         ///< function og the interface
	static constexpr ::sdv::interface_id _id = 0xA012345678900300; ///< Interface Id
};

