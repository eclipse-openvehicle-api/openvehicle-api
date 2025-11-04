#include "includes.h"
#include "generated/array.h"
#include "generated/ps/test_proxy.cpp"
#include "generated/ps/test_stub.cpp"
#include "generated/test.h"
#include "generated/union.h"
#include "generator_test.h"

class CTestResult
    : public IVarTest
    , public sdv::IInterfaceAccess
{
public:
    /**
     * @brief Get access to another interface. Overload of sdv::IInterfaceAccess::GetInterface.
     * @param[in] tInterfaceId The interface id to get access to.
     * @return Returns a pointer to the interface or NULL when the interface is not supported.
     */
    virtual sdv::interface_t GetInterface(sdv::interface_id tInterfaceId) override
    {
        if (tInterfaceId == sdv::GetInterfaceId<sdv::IInterfaceAccess>())
            return static_cast<IInterfaceAccess*>(this);
        if (tInterfaceId == sdv::GetInterfaceId<IVarTest>())
            return static_cast<IVarTest*>(this);
        return nullptr;
    }

    /** Implementation of SetFixedInt. */
    virtual void SetFixedInt(/*in*/ char i8Val, /*in*/ int16_t i16Val, /*in*/ int32_t i32Val, /*in*/ int64_t i64Val) override
    {
        m_i8Val	 = i8Val;
        m_i16Val = i16Val;
        m_i32Val = i32Val;
        m_i64Val = i64Val;
    }

    /** Implementation of GetFixedInt. */
    virtual void GetFixedInt(/*out*/ char&	  i8Val,
                             /*out*/ int16_t& i16Val,
                             /*out*/ int32_t& i32Val,
                             /*out*/ int64_t& i64Val) const override
    {
        i8Val  = m_i8Val;
        i16Val = m_i16Val;
        i32Val = m_i32Val;
        i64Val = m_i64Val;
    }

    /** Implementation of SetFixedUInt. */
    virtual void SetFixedUInt(/*in*/ bool	  bVal,
                              /*in*/ size_t	  nVal,
                              /*in*/ uint8_t  ui8Val,
                              /*in*/ uint16_t ui16Val,
                              /*in*/ uint32_t ui32Val,
                              /*in*/ uint64_t ui64Val) override
    {
        m_bVal	  = bVal;
        m_nVal	  = nVal;
        m_ui8Val  = ui8Val;
        m_ui16Val = ui16Val;
        m_ui32Val = ui32Val;
        m_ui64Val = ui64Val;
    }

    /** Implementation of GetFixedUInt. */
    virtual void GetFixedUInt(/*out*/ bool&		bVal,
                              /*out*/ size_t&	nVal,
                              /*out*/ uint8_t&	ui8Val,
                              /*out*/ uint16_t& ui16Val,
                              /*out*/ uint32_t& ui32Val,
                              /*out*/ uint64_t& ui64Val) const override
    {
        bVal	= m_bVal;
        nVal	= m_nVal;
        ui8Val	= m_ui8Val;
        ui16Val = m_ui16Val;
        ui32Val = m_ui32Val;
        ui64Val = m_ui64Val;
    }

    /** Implementation of SetFixedChar. */
    virtual void SetFixedChar(/*in*/ char cVal, /*in*/ wchar_t cwVal, /*in*/ char16_t c16Val, /*in*/ char32_t c32Val) override
    {
        m_cVal	 = cVal;
        m_cwVal	 = cwVal;
        m_c16Val = c16Val;
        m_c32Val = c32Val;
    }

    /** Implementation of GetFixedChar. */
    virtual void GetFixedChar(/*out*/ char&		cVal,
                              /*out*/ wchar_t&	cwVal,
                              /*out*/ char16_t& c16Val,
                              /*out*/ char32_t& c32Val) const override
    {
        cVal   = m_cVal;
        cwVal  = m_cwVal;
        c16Val = m_c16Val;
        c32Val = m_c32Val;
    }

    /** Implementation of SetFloatingPoint. */
    virtual void SetFloatingPoint(/*in*/ float fVal, /*in*/ double dVal/*, long double ldVal*/) override
    {
        m_fVal	= fVal;
        m_dVal	= dVal;
    // GCC issue with generated initialization in the consructor for the "long double" type: BUG #3982727
        // m_ldVal = ldVal;
    }

    /** Implementation of GetFloatingPoint. */
    virtual void GetFloatingPoint(/*out*/ float& fVal, /*out*/ double& dVal/*, long double& ldVal*/) const override
    {
        fVal  = m_fVal;
        dVal  = m_dVal;
    // GCC issue with generated initialization in the consructor for the "long double" type: BUG #3982727
        // ldVal = m_ldVal;
    }

    virtual void SetFixedPoint(/*in*/ uint32_t /*fixVal10*/, /*in*/ uint32_t /*fixVal8*/, /*in*/ uint32_t /*fixVal5*/) override {}
    virtual void GetFixedPoint(/*out*/ uint32_t& /*fixVal10*/,
                               /*out*/ uint32_t& /*fixVal8*/,
                               /*out*/ uint32_t& /*fixVal5*/) const override{};

    /** Implementation of SetString. */
    virtual void SetString(/*in*/ const sdv::string&	ssText,
                           /*in*/ const sdv::u8string&	ss8Text,
                           /*in*/ const sdv::u16string& ss16Text,
                           /*in*/ const sdv::u32string& ss32Text,
                           /*in*/ const sdv::wstring&	sswText) override
    {
        m_ssText   = ssText;
        m_ss8Text  = ss8Text;
        m_ss16Text = ss16Text;
        m_ss32Text = ss32Text;
        m_sswText  = sswText;
    }

    /** Implementation of GetString. */
    virtual void GetString(/*out*/ sdv::string&	   ssText,
                           /*out*/ sdv::u8string&  ss8Text,
                           /*out*/ sdv::u16string& ss16Text,
                           /*out*/ sdv::u32string& ss32Text,
                           /*out*/ sdv::wstring&   sswText) const override
    {
        ssText	 = m_ssText;
        ss8Text	 = m_ss8Text;
        ss16Text = m_ss16Text;
        ss32Text = m_ss32Text;
        sswText	 = m_sswText;
    }

    virtual void SetFixedString(/*in*/ const sdv::fixed_string<10>&	   ssText,
                                /*in*/ const sdv::fixed_u8string<10>&  ss8Text,
                                /*in*/ const sdv::fixed_u16string<10>& ss16Text,
                                /*in*/ const sdv::fixed_u32string<10>& ss32Text,
                                /*in*/ const sdv::fixed_wstring<10>&   sswText) override
    {
        m_ssText   = ssText;
        m_ss8Text  = ss8Text;
        m_ss16Text = ss16Text;
        m_ss32Text = ss32Text;
        m_sswText  = sswText;
    }

    virtual void GetFixedString(/*out*/ sdv::fixed_string<15>&	  ssText,
                                /*out*/ sdv::fixed_u8string<15>&  ss8Text,
                                /*out*/ sdv::fixed_u16string<15>& ss16Text,
                                /*out*/ sdv::fixed_u32string<15>& ss32Text,
                                /*out*/ sdv::fixed_wstring<15>&	  sswText) const override
    {
        ssText	 = m_ssText;
        ss8Text	 = m_ss8Text;
        ss16Text = m_ss16Text;
        ss32Text = m_ss32Text;
        sswText	 = m_sswText;
    }

    /**
     * @{
     * @brief Test sequence.
     */
    virtual void SetSequence(/*in*/ const sdv::sequence<uint32_t>&				  seqUInts,
                             /*in*/ const sdv::sequence<sdv::string>&			  seqTexts,
                             /*in*/ const sdv::sequence<sdv::sequence<uint32_t>>& seqSequences) override
    {
        m_seqUInts	   = seqUInts;
        m_seqTexts	   = seqTexts;
        m_seqSequences = seqSequences;
    }

    virtual void GetSequence(/*out*/ sdv::sequence<uint32_t>&				 seqUInts,
                             /*out*/ sdv::sequence<sdv::string>&			 seqTexts,
                             /*out*/ sdv::sequence<sdv::sequence<uint32_t>>& seqSequences) override
    {
        seqUInts	 = m_seqUInts;
        seqTexts	 = m_seqTexts;
        seqSequences = m_seqSequences;
    }

    /**
     * @{
     * @brief Test fixed size sequence.
     */
    virtual void SetFixedSequence(/*in*/ const sdv::sequence<uint32_t, 10>&	   seqUInts,
                                  /*in*/ const sdv::sequence<sdv::string, 10>& seqTexts,
                                  /*in*/ const sdv::sequence<sdv::sequence<uint32_t, 10>, 2>& /*seqSequences*/) override
    {
        m_seqUInts = seqUInts;
        m_seqTexts = seqTexts;
        // TODO...
        // m_seqSequences = seqSequences;
    }

    virtual void GetFixedSequence(/*out*/ sdv::sequence<uint32_t, 7>&	  seqUInts,
                                  /*out*/ sdv::sequence<sdv::string, 15>& seqTexts,
                                  /*out*/ sdv::sequence<sdv::sequence<uint32_t, 5>, 3>& /*seqSequences*/) override
    {
        seqUInts = m_seqUInts;
        seqTexts = m_seqTexts;
        // TODO...
        // seqSequences = m_seqSequences;
    }

    /**
     * @{
     * @brief Test map.
     */
    virtual void SetMap(/*in*/ uint32_t /*mapUInts*/, /*in*/ uint32_t /*mapTexts*/, /*in*/ uint32_t /*mapSequences*/) override {}

    virtual void GetMap(/*out*/ uint32_t& /*mapUInts*/, /*out*/ uint32_t& /*mapTexts*/, /*out*/ uint32_t& /*mapSequences*/) override
    {}

    /**
     * @{
     * @brief Test fixed size map.
     */
    virtual void SetFixedMap(/*in*/ uint32_t /*mapUInts*/, /*in*/ uint32_t /*mapTexts*/, /*in*/ uint32_t /*mapSequences*/) override
    {}

    virtual void GetFixedMap(/*out*/ uint32_t& /*mapUInts*/,
                             /*out*/ uint32_t& /*mapTexts*/,
                             /*out*/ uint32_t& /*mapSequences*/) override
    {}

    /**
     * @{
     * @brief Test pointer.
     */
    virtual void SetPointer(/*in*/ const sdv::pointer<uint32_t>&				ptrUInts,
                            /*in*/ const sdv::pointer<sdv::string>&				ptrTexts,
                            /*in*/ const sdv::pointer<sdv::sequence<uint32_t>>& ptrSequences) override
    {
        m_ptrUInts	   = ptrUInts;
        m_ptrTexts	   = ptrTexts;
        m_ptrSequences = ptrSequences;
    }

    virtual void GetPointer(/*out*/ sdv::pointer<uint32_t>&				   ptrUInts,
                            /*out*/ sdv::pointer<sdv::string>&			   ptrTexts,
                            /*out*/ sdv::pointer<sdv::sequence<uint32_t>>& ptrSequences) override
    {
        ptrUInts	 = m_ptrUInts;
        ptrTexts	 = m_ptrTexts;
        ptrSequences = m_ptrSequences;
    }

    /**
     * @{
     * @brief Test fixed size pointer.
     */
    virtual void SetFixedPointer(/*in*/ const sdv::pointer<uint32_t, 10>&	 ptrUInts,
                                 /*in*/ const sdv::pointer<sdv::string, 10>& ptrTexts,
                                 /*in*/ const sdv::pointer<sdv::sequence<uint32_t, 10>, 2>& /*ptrSequences*/) override
    {
        m_ptrUInts = ptrUInts;
        m_ptrTexts = ptrTexts;
        // TODO...
        // m_ptrSequences = ptrSequences;
    }

    virtual void GetFixedPointer(/*out*/ sdv::pointer<uint32_t, 7>&		ptrUInts,
                                 /*out*/ sdv::pointer<sdv::string, 15>& ptrTexts,
                                 /*out*/ sdv::pointer<sdv::sequence<uint32_t, 5>, 3>& /*ptrSequences*/) override
    {
        ptrUInts = m_ptrUInts;
        ptrTexts = m_ptrTexts;
        // TODO...
        // ptrSequences = m_ptrSequences;
    }

    /**
     * @{
     * @brief Test any data type.
     */
    virtual void SetAny(/*in*/ sdv::any_t anyMyValue) override
    {
        m_anyVal = anyMyValue;
    }

    virtual void GetAny(/*out*/ sdv::any_t& anyMyValue) override
    {
        anyMyValue = m_anyVal;
    }

    virtual void SetComplex(/*in*/ const SComplex& rsComplex)
    {
        m_sComplex = rsComplex;
    }

    virtual SComplex GetComplex() const
    {
        return m_sComplex;
    }

    virtual void UpdateComplex(/*inout*/ SComplex& rsComplex)
    {
        std::swap(m_sComplex, rsComplex);
    }

    /** Implementation of get_bVal. */
    virtual bool get_bVal() const override
    {
        return m_bVal;
    }

    /** Implementation of set_bVal. */
    virtual void set_bVal(bool bVal) override
    {
        m_bVal = bVal;
    }

    /** Implementation of get_nVal. */
    virtual size_t get_nVal() const override
    {
        return m_nVal;
    }

    /** Implementation of set_nVal. */
    virtual void set_nVal(size_t nVal) override
    {
        m_nVal = nVal;
    }

    /** Implementation of get_i8Val. */
    virtual char get_i8Val() const override
    {
        return m_i8Val;
    }

    /** Implementation of set_i8Val. */
    virtual void set_i8Val(char i8Val) override
    {
        m_i8Val = i8Val;
    }

    /** Implementation of get_i16Val. */
    virtual int16_t get_i16Val() const override
    {
        return m_i16Val;
    }

    /** Implementation of set_i16Val. */
    virtual void set_i16Val(int16_t i16Val) override
    {
        m_i16Val = i16Val;
    }

    /** Implementation of get_i32Val. */
    virtual int32_t get_i32Val() const override
    {
        return m_i32Val;
    }

    /** Implementation of set_i32Val. */
    virtual void set_i32Val(int32_t i32Val) override
    {
        m_i32Val = i32Val;
    }

    /** Implementation of get_i64Val. */
    virtual int64_t get_i64Val() const override
    {
        return m_i64Val;
    }

    /** Implementation of set_i64Val. */
    virtual void set_i64Val(int64_t i64Val) override
    {
        m_i64Val = i64Val;
    }

    /** Implementation of get_ui8Val. */
    virtual uint8_t get_ui8Val() const override
    {
        return m_ui8Val;
    }

    /** Implementation of set_ui8Val. */
    virtual void set_ui8Val(uint8_t ui8Val) override
    {
        m_ui8Val = ui8Val;
    }

    /** Implementation of get_ui16Val. */
    virtual uint16_t get_ui16Val() const override
    {
        return m_ui16Val;
    }

    /** Implementation of set_ui16Val. */
    virtual void set_ui16Val(uint16_t ui16Val) override
    {
        m_ui16Val = ui16Val;
    }

    /** Implementation of get_ui32Val. */
    virtual uint32_t get_ui32Val() const override
    {
        return m_ui32Val;
    }

    /** Implementation of set_ui32Val. */
    virtual void set_ui32Val(uint32_t ui32Val) override
    {
        m_ui32Val = ui32Val;
    }

    /** Implementation of get_ui64Val. */
    virtual uint64_t get_ui64Val() const override
    {
        return m_ui64Val;
    }

    /** Implementation of set_ui64Val. */
    virtual void set_ui64Val(uint64_t ui64Val) override
    {
        m_ui64Val = ui64Val;
    }

    /** Implementation of get_cVal. */
    virtual char get_cVal() const override
    {
        return m_cVal;
    }

    /** Implementation of set_cVal. */
    virtual void set_cVal(char cVal) override
    {
        m_cVal = cVal;
    }

    /** Implementation of get_cwVal. */
    virtual wchar_t get_cwVal() const override
    {
        return m_cwVal;
    }

    /** Implementation of set_cwVal. */
    virtual void set_cwVal(wchar_t cwVal) override
    {
        m_cwVal = cwVal;
    }

    /** Implementation of get_c16Val. */
    virtual char16_t get_c16Val() const override
    {
        return m_c16Val;
    }

    /** Implementation of set_c16Val. */
    virtual void set_c16Val(char16_t c16Val) override
    {
        m_c16Val = c16Val;
    }

    /** Implementation of get_c32Val. */
    virtual char32_t get_c32Val() const override
    {
        return m_c32Val;
    }

    /** Implementation of set_c32Val. */
    virtual void set_c32Val(char32_t c32Val) override
    {
        m_c32Val = c32Val;
    }

    /** Implementation of get_fVal. */
    virtual float get_fVal() const override
    {
        return m_fVal;
    }

    /** Implementation of set_fVal. */
    virtual void set_fVal(float fVal) override
    {
        m_fVal = fVal;
    }

    /** Implementation of get_dVal. */
    virtual double get_dVal() const override
    {
        return m_dVal;
    }

    /** Implementation of set_dVal. */
    virtual void set_dVal(double dVal) override
    {
        m_dVal = dVal;
    }

    ///** Implementation of get_ldVal. */
    //virtual long double get_ldVal() const override
    //{
    //    return m_ldVal;
    //}

    ///** Implementation of set_ldVal. */
    //virtual void set_ldVal(long double ldVal) override
    //{
    //    m_ldVal = ldVal;
    //}

    virtual uint32_t get_fixVal10() const override
    {
        return 0;
    }
    virtual void set_fixVal10(uint32_t /*attr*/) override {}

    virtual uint32_t get_fixVal8() const override
    {
        return 0;
    }
    virtual void set_fixVal8(uint32_t /*attr*/) override {}

    virtual uint32_t get_fixVal5() const override
    {
        return 0;
    }
    virtual void set_fixVal5(uint32_t /*attr*/) override {}

    /** Implementation of get_ssText. */
    virtual sdv::string get_ssText() const override
    {
        return m_ssText;
    }

    /** Implementation of set_ssText. */
    virtual void set_ssText(const sdv::string& ssText) override
    {
        m_ssText = ssText;
    }

    /** Implementation of get_ss8Text. */
    virtual sdv::u8string get_ss8Text() const override
    {
        return m_ss8Text;
    }

    /** Implementation of set_ss8Text. */
    virtual void set_ss8Text(const sdv::u8string& ss8Text) override
    {
        m_ss8Text = ss8Text;
    }

    /** Implementation of get_ss16Text. */
    virtual sdv::u16string get_ss16Text() const override
    {
        return m_ss16Text;
    }

    /** Implementation of set_ss16Text. */
    virtual void set_ss16Text(const sdv::u16string& ss16Text) override
    {
        m_ss16Text = ss16Text;
    }

    /** Implementation of get_ss32Text. */
    virtual sdv::u32string get_ss32Text() const override
    {
        return m_ss32Text;
    }

    /** Implementation of set_ss32Text. */
    virtual void set_ss32Text(const sdv::u32string& ss32Text) override
    {
        m_ss32Text = ss32Text;
    }

    /** Implementation of get_sswText. */
    virtual sdv::wstring get_sswText() const override
    {
        return m_sswText;
    }

    /** Implementation of set_sswText. */
    virtual void set_sswText(const sdv::wstring& sswText) override
    {
        m_sswText = sswText;
    }

    virtual sdv::fixed_string<10> get_ssFixedText() const override
    {
        return sdv::fixed_string<10>();
    }
    virtual void set_ssFixedText(const sdv::fixed_string<10>& /*attr*/) override {}

    virtual sdv::fixed_u8string<10> get_ss8FixedText() const override
    {
        return sdv::fixed_u8string<10>();
    }
    virtual void set_ss8FixedText(const sdv::fixed_u8string<10>& /*attr*/) override {}

    virtual sdv::fixed_u16string<10> get_ss16FixedText() const override
    {
        return sdv::fixed_u16string<10>();
    }
    virtual void set_ss16FixedText(const sdv::fixed_u16string<10>& /*attr*/) override {}

    virtual sdv::fixed_u32string<10> get_ss32FixedText() const override
    {
        return sdv::fixed_u32string<10>();
    }
    virtual void set_ss32FixedText(const sdv::fixed_u32string<10>& /*attr*/) override {}

    virtual sdv::fixed_wstring<10> get_sswFixedText() const override
    {
        return sdv::fixed_wstring<10>();
    }
    virtual void set_sswFixedText(const sdv::fixed_wstring<10>& /*attr*/) override {}

    virtual sdv::sequence<uint32_t> get_seqUInts() const override
    {
        return sdv::sequence<uint32_t>();
    }
    virtual void set_seqUInts(const sdv::sequence<uint32_t>& /*attr*/) override {}

    virtual sdv::sequence<sdv::string> get_seqTexts() const override
    {
        return sdv::sequence<sdv::string>();
    }
    virtual void set_seqTexts(const sdv::sequence<sdv::string>& /*attr*/) override {}

    virtual sdv::sequence<sdv::sequence<uint32_t>> get_seqSequences() const override
    {
        return sdv::sequence<sdv::sequence<uint32_t>>();
    }
    virtual void set_seqSequences(const sdv::sequence<sdv::sequence<uint32_t>>& /*attr*/) override {}

    virtual sdv::sequence<uint32_t, 10> get_seqFixedUInts() const override
    {
        return sdv::sequence<uint32_t, 10>();
    }
    virtual void set_seqFixedUInts(const sdv::sequence<uint32_t, 10>& /*attr*/) override {}

    virtual sdv::sequence<sdv::string, 10> get_seqFixedTexts() const override
    {
        return sdv::sequence<sdv::string, 10>();
    }
    virtual void set_seqFixedTexts(const sdv::sequence<sdv::string, 10>& /*attr*/) override {}

    virtual sdv::sequence<sdv::sequence<uint32_t, 10>, 2> get_seqFixedSequences() const override
    {
        return sdv::sequence<sdv::sequence<uint32_t, 10>, 2>();
    }
    virtual void set_seqFixedSequences(const sdv::sequence<sdv::sequence<uint32_t, 10>, 2>& /*attr*/) override {}

    virtual uint32_t get_mapUInts() const override
    {
        return 0;
    }
    virtual void set_mapUInts(uint32_t /*attr*/) override {}

    virtual uint32_t get_mapTexts() const override
    {
        return 0;
    }
    virtual void set_mapTexts(uint32_t /*attr*/) override {}

    virtual uint32_t get_mapSequences() const override
    {
        return 0;
    }
    virtual void set_mapSequences(uint32_t /*attr*/) override {}

    virtual uint32_t get_mapFixedUInts() const override
    {
        return 0;
    }
    virtual void set_mapFixedUInts(uint32_t /*attr*/) override {}

    virtual uint32_t get_mapFixedTexts() const override
    {
        return 0;
    }
    virtual void set_mapFixedTexts(uint32_t /*attr*/) override {}

    virtual uint32_t get_mapFixedSequences() const override
    {
        return 0;
    }
    virtual void set_mapFixedSequences(uint32_t /*attr*/) override {}

    virtual sdv::pointer<uint32_t> get_ptrUInts() const override
    {
        return sdv::pointer<uint32_t>();
    }
    virtual void set_ptrUInts(const sdv::pointer<uint32_t>& /*attr*/) override {}

    virtual sdv::pointer<sdv::string> get_ptrTexts() const override
    {
        return sdv::pointer<sdv::string>();
    }
    virtual void set_ptrTexts(const sdv::pointer<sdv::string>& /*attr*/) override {}

    virtual sdv::pointer<sdv::sequence<uint32_t>> get_ptrSequences() const override
    {
        return sdv::pointer<sdv::sequence<uint32_t>>();
    }
    virtual void set_ptrSequences(const sdv::pointer<sdv::sequence<uint32_t>>& /*attr*/) override {}

    virtual sdv::pointer<uint32_t, 10> get_ptrFixedUInts() const override
    {
        return sdv::pointer<uint32_t, 10>();
    }
    virtual void set_ptrFixedUInts(const sdv::pointer<uint32_t, 10>& /*attr*/) override {}

    virtual sdv::pointer<sdv::string, 10> get_ptrFixedTexts() const override
    {
        return sdv::pointer<sdv::string, 10>();
    }
    virtual void set_ptrFixedTexts(const sdv::pointer<sdv::string, 10>& /*attr*/) override {}

    virtual sdv::pointer<sdv::sequence<uint32_t, 10>, 2> get_ptrFixedSequences() const override
    {
        return sdv::pointer<sdv::sequence<uint32_t, 10>, 2>();
    }
    virtual void set_ptrFixedSequences(const sdv::pointer<sdv::sequence<uint32_t, 10>, 2>& /*attr*/) override {}

    virtual sdv::any_t get_anyMyValue() const override
    {
        return m_anyVal;
    }
    virtual void set_anyMyValue(sdv::any_t attr) override
    {
        m_anyVal = attr;
    }

    /**
    * Trigger complex exception.
    */
    virtual void TriggerComplexException() override { XExceptionTest exception; exception.sComplex = m_sComplex; throw exception; }

    /**
    * Trigger system exception.
    */
    virtual void TriggerSystemException() override { sdv::XNullPointer exception; throw exception; }

    /**
    * Trigger unhandled exception.
    */
    virtual void TriggerUnhandledException() override { XExceptionTest exception; exception.sComplex = m_sComplex; throw exception; }

#ifdef _MSC_VER
    // Prevent static code analysis for dereferencing NULL pointer.
    #pragma warning(push)
    #pragma warning(disable : 6011)
#endif

    /**
    * Trigger crash exception.
    */
    void TriggerCrashException() override
	{
		int* p = nullptr;
		*p = 10;
	}

#ifdef _MSC_VER
    #pragma warning(pop)
#endif

private:
    bool								   m_bVal	 = false;
    size_t								   m_nVal	 = 0;
    int8_t								   m_i8Val	 = 0;
    int16_t								   m_i16Val	 = 0;
    int32_t								   m_i32Val	 = 0;
    int64_t								   m_i64Val	 = 0;
    uint8_t								   m_ui8Val	 = 0;
    uint16_t							   m_ui16Val = 0;
    uint32_t							   m_ui32Val = 0;
    uint64_t							   m_ui64Val = 0;
    char								   m_cVal	 = 0;
    wchar_t								   m_cwVal	 = 0;
    char16_t							   m_c16Val	 = 0;
    char32_t							   m_c32Val	 = 0;
    float								   m_fVal	 = 0.0;
    double								   m_dVal	 = 0.0;
    long double							   m_ldVal	 = 0.0;
    std::string							   m_ssText;
    std::string							   m_ss8Text;
    std::u16string						   m_ss16Text;
    std::u32string						   m_ss32Text;
    std::wstring						   m_sswText;
    sdv::sequence<uint32_t>				   m_seqUInts;
    sdv::sequence<sdv::string>			   m_seqTexts;
    sdv::sequence<sdv::sequence<uint32_t>> m_seqSequences;
    sdv::pointer<uint32_t>				   m_ptrUInts;
    sdv::pointer<sdv::string>			   m_ptrTexts;
    sdv::pointer<sdv::sequence<uint32_t>>  m_ptrSequences;
    SComplex							   m_sComplex;
    sdv::any_t                             m_anyVal;
};

TEST_F(CGeneratorTest, Instantiation)
{
    CTestResult	   test;
    IVarTest__stub stub;
    stub.Link(static_cast<IVarTest*>(&test));
    IVarTest__proxy proxy;
    proxy.Link(&stub);
}

TEST_F(CGeneratorTest, OperationSignedIntegral)
{
    CTestResult test;
    IVarTest__stub stub;
    stub.Link(static_cast<IVarTest*>(&test));
    IVarTest__proxy proxy;
    proxy.Link(&stub);

    int8_t i8Val  = 0;
    int16_t i16Val = 0;
    int32_t i32Val = 0;
    int64_t i64Val = 0;

    proxy.Access().GetFixedInt((char&)i8Val, i16Val, i32Val, i64Val);

    EXPECT_EQ(i8Val, 0);
    EXPECT_EQ(i16Val, 0);
    EXPECT_EQ(i32Val, 0);
    EXPECT_EQ(i64Val, 0);

    proxy.Access().SetFixedInt(-10, -20, -30, -40);
    proxy.Access().GetFixedInt((char&)i8Val, i16Val, i32Val, i64Val);

    EXPECT_EQ(i8Val, -10);
    EXPECT_EQ(i16Val, -20);
    EXPECT_EQ(i32Val, -30);
    EXPECT_EQ(i64Val, -40);
}

TEST_F(CGeneratorTest, OperationUnsignedIntegral)
{
    CTestResult	   test;
    IVarTest__stub stub;
    stub.Link(static_cast<IVarTest*>(&test));
    IVarTest__proxy proxy;
    proxy.Link(&stub);

    bool	 bVal	 = false;
    size_t	 nVal	 = 0;
    uint8_t	 ui8Val	 = 0;
    uint16_t ui16Val = 0;
    uint32_t ui32Val = 0;
    uint64_t ui64Val = 0;

    proxy.Access().GetFixedUInt(bVal, nVal, ui8Val, ui16Val, ui32Val, ui64Val);

    EXPECT_EQ(bVal, false);
    EXPECT_EQ(nVal, 0);
    EXPECT_EQ(ui8Val, 0);
    EXPECT_EQ(ui16Val, 0u);
    EXPECT_EQ(ui32Val, 0u);
    EXPECT_EQ(ui64Val, 0ull);

    proxy.Access().SetFixedUInt(true, 40, 50, 60, 70, 80);
    proxy.Access().GetFixedUInt(bVal, nVal, ui8Val, ui16Val, ui32Val, ui64Val);

    EXPECT_EQ(bVal, true);
    EXPECT_EQ(nVal, 40);
    EXPECT_EQ(ui8Val, 50);
    EXPECT_EQ(ui16Val, 60);
    EXPECT_EQ(ui32Val, 70u);
    EXPECT_EQ(ui64Val, 80ull);
}

TEST_F(CGeneratorTest, OperationFloatingPoint)
{
    CTestResult	   test;
    IVarTest__stub stub;
    stub.Link(static_cast<IVarTest*>(&test));
    IVarTest__proxy proxy;
    proxy.Link(&stub);


    float		fVal  = 0.0f;
    double		dVal  = 0.0;
    // GCC issue with generated initialization in the consructor for the "long double" type: BUG #3982727
    //long double ldVal = 0.0l;

    proxy.Access().GetFloatingPoint(fVal, dVal/*, ldVal*/);

    EXPECT_EQ(fVal, 0.0);
    EXPECT_EQ(dVal, 0.0);
    //EXPECT_EQ(ldVal, 0.0l);

    proxy.Access().SetFloatingPoint(100.1f, 200.2/*, 300.3l*/);
    proxy.Access().GetFloatingPoint(fVal, dVal/*, ldVal*/);

    EXPECT_EQ(fVal, 100.1f);
    EXPECT_EQ(dVal, 200.2);
    //EXPECT_EQ(ldVal, 300.3l);
}

TEST_F(CGeneratorTest, OperationCharacter)
{
    CTestResult	   test;
    IVarTest__stub stub;
    stub.Link(static_cast<IVarTest*>(&test));
    IVarTest__proxy proxy;
    proxy.Link(&stub);


    char	 cVal	= 0;
    wchar_t	 cwVal	= 0;
    char16_t c16Val = 0;
    char32_t c32Val = 0;

    proxy.Access().GetFixedChar(cVal, cwVal, c16Val, c32Val);

    EXPECT_EQ(cVal, 0);
    EXPECT_EQ(cwVal, 0);
    EXPECT_EQ(c16Val, 0);
    EXPECT_EQ(c32Val, 0u);

    proxy.Access().SetFixedChar('a', L'b', u'c', U'd');
    proxy.Access().GetFixedChar(cVal, cwVal, c16Val, c32Val);

    EXPECT_EQ(cVal, 'a');
    EXPECT_EQ(cwVal, L'b');
    EXPECT_EQ(c16Val, u'c');
    EXPECT_EQ(c32Val, U'd');
}

TEST_F(CGeneratorTest, OperationString)
{
    CTestResult	   test;
    IVarTest__stub stub;
    stub.Link(static_cast<IVarTest*>(&test));
    IVarTest__proxy proxy;
    proxy.Link(&stub);

    sdv::string	   ssVal;
    sdv::u8string  ss8Val;
    sdv::u16string ss16Val;
    sdv::u32string ss32Val;
    sdv::wstring   sswVal;

    proxy.Access().GetString(ssVal, ss8Val, ss16Val, ss32Val, sswVal);

    EXPECT_TRUE(ssVal.empty());
    EXPECT_TRUE(ss8Val.empty());
    EXPECT_TRUE(ss16Val.empty());
    EXPECT_TRUE(ss32Val.empty());
    EXPECT_TRUE(sswVal.empty());

    proxy.Access().SetString("ansi", u8"utf-8", u"utf-16", U"utf-32", L"wide");

    proxy.Access().GetString(ssVal, ss8Val, ss16Val, ss32Val, sswVal);

    EXPECT_EQ(ssVal, "ansi");
    EXPECT_EQ(ss8Val, u8"utf-8");
    EXPECT_EQ(ss16Val, u"utf-16");
    EXPECT_EQ(ss32Val, U"utf-32");
    EXPECT_EQ(sswVal, L"wide");
}

TEST_F(CGeneratorTest, OperationAny)
{
    CTestResult	   test;
    IVarTest__stub stub;
    stub.Link(static_cast<IVarTest*>(&test));
    IVarTest__proxy proxy;
    proxy.Link(&stub);

    sdv::any_t anyVal;
    proxy.Access().GetAny(anyVal);

    EXPECT_TRUE(anyVal.empty());

    proxy.Access().SetAny(sdv::any_t("ansi"));

    proxy.Access().GetAny(anyVal);

    EXPECT_EQ(anyVal.get<std::string>(), "ansi");
}

TEST_F(CGeneratorTest, AttributeSignedIntegral)
{
    CTestResult	   test;
    IVarTest__stub stub;
    stub.Link(static_cast<IVarTest*>(&test));
    IVarTest__proxy proxy;
    proxy.Link(&stub);

    int8_t	i8Val  = proxy.Access().get_i8Val();
    int16_t i16Val = proxy.Access().get_i16Val();
    int32_t i32Val = proxy.Access().get_i32Val();
    int64_t i64Val = proxy.Access().get_i64Val();

    EXPECT_EQ(i8Val, 0);
    EXPECT_EQ(i16Val, 0);
    EXPECT_EQ(i32Val, 0);
    EXPECT_EQ(i64Val, 0);

    proxy.Access().set_i8Val(-10);
    proxy.Access().set_i16Val(-20);
    proxy.Access().set_i32Val(-30);
    proxy.Access().set_i64Val(-40);
    i8Val  = proxy.Access().get_i8Val();
    i16Val = proxy.Access().get_i16Val();
    i32Val = proxy.Access().get_i32Val();
    i64Val = proxy.Access().get_i64Val();

    EXPECT_EQ(i8Val, -10);
    EXPECT_EQ(i16Val, -20);
    EXPECT_EQ(i32Val, -30);
    EXPECT_EQ(i64Val, -40);
}

TEST_F(CGeneratorTest, AttributeUnsignedIntegral)
{
    CTestResult	   test;
    IVarTest__stub stub;
    stub.Link(static_cast<IVarTest*>(&test));
    IVarTest__proxy proxy;
    proxy.Link(&stub);

    bool	 bVal	 = proxy.Access().get_bVal();
    size_t	 nVal	 = proxy.Access().get_nVal();
    uint8_t	 ui8Val	 = proxy.Access().get_ui8Val();
    uint16_t ui16Val = proxy.Access().get_ui16Val();
    uint32_t ui32Val = proxy.Access().get_ui32Val();
    uint64_t ui64Val = proxy.Access().get_ui64Val();

    EXPECT_EQ(bVal, false);
    EXPECT_EQ(nVal, 0);
    EXPECT_EQ(ui8Val, 0);
    EXPECT_EQ(ui16Val, 0u);
    EXPECT_EQ(ui32Val, 0u);
    EXPECT_EQ(ui64Val, 0ull);

    proxy.Access().set_bVal(true);
    proxy.Access().set_nVal(40);
    proxy.Access().set_ui8Val(50);
    proxy.Access().set_ui16Val(60);
    proxy.Access().set_ui32Val(70);
    proxy.Access().set_ui64Val(80);
    bVal	= proxy.Access().get_bVal();
    nVal	= proxy.Access().get_nVal();
    ui8Val	= proxy.Access().get_ui8Val();
    ui16Val = proxy.Access().get_ui16Val();
    ui32Val = proxy.Access().get_ui32Val();
    ui64Val = proxy.Access().get_ui64Val();

    EXPECT_EQ(bVal, true);
    EXPECT_EQ(nVal, 40);
    EXPECT_EQ(ui8Val, 50);
    EXPECT_EQ(ui16Val, 60u);
    EXPECT_EQ(ui32Val, 70u);
    EXPECT_EQ(ui64Val, 80ull);
}

TEST_F(CGeneratorTest, AttributeFloatingPoint)
{
    CTestResult	   test;
    IVarTest__stub stub;
    stub.Link(static_cast<IVarTest*>(&test));
    IVarTest__proxy proxy;
    proxy.Link(&stub);

    float		fVal  = proxy.Access().get_fVal();
    double		dVal  = proxy.Access().get_dVal();
    //long double ldVal = proxy.Access().get_ldVal();

    EXPECT_EQ(fVal, 0.0f);
    EXPECT_EQ(dVal, 0.0);
    //EXPECT_EQ(ldVal, 0.0l);

    proxy.Access().set_fVal(100.1f);
    proxy.Access().set_dVal(200.2);
    //proxy.Access().set_ldVal(300.3l);
    fVal  = proxy.Access().get_fVal();
    dVal  = proxy.Access().get_dVal();
    //ldVal = proxy.Access().get_ldVal();

    EXPECT_EQ(fVal, 100.1f);
    EXPECT_EQ(dVal, 200.2);
    //EXPECT_EQ(ldVal, 300.3l);
}

TEST_F(CGeneratorTest, AttributeCharacter)
{
    CTestResult	   test;
    IVarTest__stub stub;
    stub.Link(static_cast<IVarTest*>(&test));
    IVarTest__proxy proxy;
    proxy.Link(&stub);

    char	 cVal	= proxy.Access().get_cVal();
    wchar_t	 cwVal	= proxy.Access().get_cwVal();
    char16_t c16Val = proxy.Access().get_c16Val();
    char32_t c32Val = proxy.Access().get_c32Val();

    proxy.Access().GetFixedChar(cVal, cwVal, c16Val, c32Val);

    EXPECT_EQ(cVal, 0);
    EXPECT_EQ(cwVal, 0);
    EXPECT_EQ(c16Val, 0);
    EXPECT_EQ(c32Val, 0u);

    proxy.Access().set_cVal('a');
    proxy.Access().set_cwVal('b');
    proxy.Access().set_c16Val('c');
    proxy.Access().set_c32Val('d');
    cVal   = proxy.Access().get_cVal();
    cwVal  = proxy.Access().get_cwVal();
    c16Val = proxy.Access().get_c16Val();
    c32Val = proxy.Access().get_c32Val();

    EXPECT_EQ(cVal, 'a');
    EXPECT_EQ(cwVal, L'b');
    EXPECT_EQ(c16Val, u'c');
    EXPECT_EQ(c32Val, U'd');
}

TEST_F(CGeneratorTest, AttributeString)
{
    CTestResult	   test;
    IVarTest__stub stub;
    stub.Link(static_cast<IVarTest*>(&test));
    IVarTest__proxy proxy;
    proxy.Link(&stub);

    sdv::string	   ssVal   = proxy.Access().get_ssText();
    sdv::u8string  ss8Val  = proxy.Access().get_ss8Text();
    sdv::u16string ss16Val = proxy.Access().get_ss16Text();
    sdv::u32string ss32Val = proxy.Access().get_ss32Text();
    sdv::wstring   sswVal  = proxy.Access().get_sswText();

    EXPECT_TRUE(ssVal.empty());
    EXPECT_TRUE(ss8Val.empty());
    EXPECT_TRUE(ss16Val.empty());
    EXPECT_TRUE(ss32Val.empty());
    EXPECT_TRUE(sswVal.empty());

    proxy.Access().set_ssText("ansi");
    proxy.Access().set_ss8Text(u8"utf-8");
    proxy.Access().set_ss16Text(u"utf-16");
    proxy.Access().set_ss32Text(U"utf-32");
    proxy.Access().set_sswText(L"wide");

    ssVal	= proxy.Access().get_ssText();
    ss8Val	= proxy.Access().get_ss8Text();
    ss16Val = proxy.Access().get_ss16Text();
    ss32Val = proxy.Access().get_ss32Text();
    sswVal	= proxy.Access().get_sswText();

    EXPECT_EQ(ssVal, "ansi");
    EXPECT_EQ(ss8Val, u8"utf-8");
    EXPECT_EQ(ss16Val, u"utf-16");
    EXPECT_EQ(ss32Val, U"utf-32");
    EXPECT_EQ(sswVal, L"wide");
}

TEST_F(CGeneratorTest, AttributeAny)
{
    CTestResult	   test;
    IVarTest__stub stub;
    stub.Link(static_cast<IVarTest*>(&test));
    IVarTest__proxy proxy;
    proxy.Link(&stub);

    sdv::any_t anyVal = proxy.Access().get_anyMyValue();

    EXPECT_TRUE(anyVal.empty());

    proxy.Access().set_anyMyValue(sdv::any_t("ansi"));

    anyVal = proxy.Access().get_anyMyValue();

    EXPECT_EQ(anyVal.get<std::string>(), "ansi");
}

TEST_F(CGeneratorTest, ComplexStructure)
{
    CTestResult	   test;
    IVarTest__stub stub;
    stub.Link(static_cast<IVarTest*>(&test));
    IVarTest__proxy proxy;
    proxy.Link(&stub);

    SComplex complex = proxy.Access().GetComplex();
    EXPECT_EQ(complex.bVal, true);
    EXPECT_TRUE(complex.ss16Val.empty());
    EXPECT_TRUE(complex.seqRgbVal.empty());
    complex.bVal	  = false;
    complex.ss16Val	  = u"This is a new value...";
    complex.seqRgbVal = {{"abc", "def", "ghi"}, {"aaa", "bbb", "ccc"}};
    complex.anyString = U"My long string";
    complex.anyFloat = 1234.5678f;
    complex.anyInteger = 1234567890;
    proxy.Access().SetComplex(complex);
    complex = SComplex();
    EXPECT_EQ(complex.bVal, true);
    EXPECT_TRUE(complex.ss16Val.empty());
    EXPECT_TRUE(complex.seqRgbVal.empty());
    complex = proxy.Access().GetComplex();
    EXPECT_EQ(complex.bVal, false);
    EXPECT_EQ(complex.ss16Val, u"This is a new value...");
    EXPECT_EQ(complex.seqRgbVal.size(), 2);
    EXPECT_EQ(complex.anyString.get<sdv::u32string>(), U"My long string");
    EXPECT_EQ(static_cast<float>(complex.anyFloat), 1234.5678f);
    EXPECT_EQ(complex.anyInteger.get<int64_t>(), 1234567890);
}

TEST_F(CGeneratorTest, ComplexException)
{
    CTestResult	   test;
    IVarTest__stub stub;
    stub.Link(static_cast<IVarTest*>(&test));
    IVarTest__proxy proxy;
    proxy.Link(&stub);

    SComplex complex = proxy.Access().GetComplex();
    EXPECT_EQ(complex.bVal, true);
    EXPECT_TRUE(complex.ss16Val.empty());
    EXPECT_TRUE(complex.seqRgbVal.empty());
    complex.bVal	  = false;
    complex.ss16Val	  = u"This is a new value...";
    complex.seqRgbVal = {{"abc", "def", "ghi"}, {"aaa", "bbb", "ccc"}};
    proxy.Access().SetComplex(complex);

    bool bExceptionTriggered = false;
    try
    {
        proxy.Access().TriggerComplexException();
    } catch (const XExceptionTest& rexcept)
    {
        bExceptionTriggered = true;
        EXPECT_EQ(rexcept.sComplex.bVal, false);
        EXPECT_EQ(rexcept.sComplex.ss16Val, u"This is a new value...");
        EXPECT_EQ(rexcept.sComplex.seqRgbVal.size(), 2);
    }
    EXPECT_TRUE(bExceptionTriggered);
}

TEST_F(CGeneratorTest, SystemException)
{
    CTestResult	   test;
    IVarTest__stub stub;
    stub.Link(static_cast<IVarTest*>(&test));
    IVarTest__proxy proxy;
    proxy.Link(&stub);

    bool bExceptionTriggered = false;
    try
    {
        proxy.Access().TriggerSystemException();
    } catch (const sdv::XNullPointer&)
    {
        bExceptionTriggered = true;
    }
    EXPECT_TRUE(bExceptionTriggered);

    bExceptionTriggered = false;
    try
    {
        proxy.Access().TriggerSystemException();
    } catch (const sdv::XSysExcept& rexcept)
    {
        EXPECT_EQ(rexcept.id(), sdv::GetExceptionId<sdv::XNullPointer>());
        bExceptionTriggered = true;
    }
    EXPECT_TRUE(bExceptionTriggered);
}

TEST_F(CGeneratorTest, UnhandledException)
{
    CTestResult	   test;
    IVarTest__stub stub;
    stub.Link(static_cast<IVarTest*>(&test));
    IVarTest__proxy proxy;
    proxy.Link(&stub);

    bool bExceptionTriggered = false;
    try
    {
        proxy.Access().TriggerUnhandledException();
    } catch (const sdv::XUnhandledException&)
    {
        bExceptionTriggered = true;
    }
    EXPECT_TRUE(bExceptionTriggered);
}

// This is an OS feature an will only work on Windows with SEH-Exception rerouting to C++ exceptions (/EHa - which overrides the
// default /EHs). On Linux there is no solution yet.
TEST_F(CGeneratorTest, DISABLED_CrashException)
{
    CTestResult	   test;
    IVarTest__stub stub;
    stub.Link(static_cast<IVarTest*>(&test));
    IVarTest__proxy proxy;
    proxy.Link(&stub);

    bool bExceptionTriggered = false;
    try
    {
        proxy.Access().TriggerCrashException();
    } catch (const sdv::XUnhandledException&)
    {
        bExceptionTriggered = true;
    }
    EXPECT_TRUE(bExceptionTriggered);
}

