/*!
 * \file DACParameters.h
 * \brief Definition of DACParameters class.
 */

#pragma once

#include "BaseConfig.h"

class TestRoc;

/*!
 * \brief The class represents the DAC settings of a readout chip (ROC)
 */
class DACParameters : public psi::BaseConfig {
public:
    enum Register {
        Vdig = 1, Vana = 2, Vsf = 3, Vcomp = 4, Vleak_comp = 5, VrgPr = 6, VwllPr = 7, VrgSh = 8, VwllSh = 9,
        VhldDel = 10, Vtrim = 11, VthrComp = 12, VIBias_Bus = 13, Vbias_sf = 14, VoffsetOp = 15, VIbiasOp = 16,
        VOffsetR0 = 17, VIon = 18, VIbias_PH = 19, Ibias_DAC = 20, VIbias_roc = 21, VIColOr = 22, Vnpix = 23,
        VSumCol = 24, Vcal = 25, CalDel = 26, RangeTemp = 27, CtrlReg = 253, WBC = 254
    };

    static const std::string& GetRegisterName(Register reg);

public:
    void Apply(TestRoc& roc, bool correction);
    void Set(TestRoc& roc, Register reg, int value, bool correction = true);
    int Get(Register reg) const;

    virtual void Read(const std::string& fileName);
    virtual void Write(const std::string& fileName) const;

private:
    struct Descriptor {
        std::string name;
        bool resetRequired, hasCalibrationTable;
        unsigned delay;

        explicit Descriptor() : name(""), resetRequired(false), hasCalibrationTable(false), delay(1000) {}
        explicit Descriptor(const std::string& aName)
            : name(aName), resetRequired(false), hasCalibrationTable(false), delay(1000) {}
        Descriptor(const std::string& aName, unsigned aDelay, bool _hasCalibrationTable = false,
                   bool _resetRequired = false)
            : name(aName), resetRequired(_resetRequired), hasCalibrationTable(_hasCalibrationTable), delay(aDelay) {}
    };

    typedef std::map<Register, Descriptor> DescriptorMap;
    static const DescriptorMap& Descriptors();
    static const Descriptor& FindDescriptor(Register reg);
};

extern std::istream& operator>>(std::istream& s, DACParameters::Register& reg);
