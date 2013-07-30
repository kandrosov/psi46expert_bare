/*!
 * \file SmartTree.h
 * \brief Definition of SmartTree class.
 * \author Konstantin Androsov <konstantin.androsov@gmail.com>
 */

#pragma once

#include <stdexcept>

#ifdef SMART_TREE_FOR_CMSSW
#include "FWCore/ServiceRegistry/interface/Service.h"
#include "CommonTools/UtilAlgos/interface/TFileService.h"
#else
#include "TFile.h"
#endif

#include "TTree.h"

#define DEFINE_TREE_DATA_ACCESSOR(type) \
    template<> \
    struct DataAccessor<type> { \
        typedef std::vector<type>* p_vector; \
        typedef type* p_type; \
        static bool HaveSimpleValue(const SmartTreeData& data, const std::string& name) \
        { \
            return data.type##_simple.find(name) != data.type##_simple.end(); \
        } \
        static p_type& GetSimpleValue(SmartTreeData& data, const std::string& name) \
        { \
            return data.type##_simple[name]; \
        } \
        static bool HaveVectorValue(const SmartTreeData& data, const std::string& name) \
        { \
            return data.type##_vector.find(name) != data.type##_vector.end(); \
        } \
        static p_vector& GetVectorValue(SmartTreeData& data, const std::string& name) \
        { \
            return data.type##_vector[name]; \
        } \
    };

#define DEFINE_SMART_TREE_DATA_CONTAINER(type) \
    std::map< std::string, type* > type##_simple; \
    std::map< std::string, std::vector<type>* > type##_vector;

#define CLEAR_SMART_TREE_DATA_CONTAINER(type) \
    for(std::map< std::string, std::vector<type>* >::iterator iter = type##_vector.begin(); \
                                                              iter != type##_vector.end(); ++iter) { \
        iter->second->clear(); }

#define DELETE_SMART_TREE_DATA_CONTAINER(type) \
    for(std::map< std::string, std::vector<type>* >::iterator iter = type##_vector.begin(); \
                                                              iter != type##_vector.end(); ++iter) { \
        delete iter->second; } \
    for(std::map< std::string, type* >::iterator iter = type##_simple.begin(); \
                                                 iter != type##_simple.end(); ++iter) { \
        delete iter->second; }

#define SIMPLE_TREE_BRANCH(type, name, default_value) \
    type& name() \
    { \
        if(!data.HaveSimpleValue< type >(#name)) { \
            const type def_val(default_value); \
            AddSimpleBranch< type >(#name, &def_val); \
        } \
        return *data.GetSimpleValue< type >(#name); \
    }

#define POINTER_TREE_BRANCH(type, name) \
    type& name() \
    { \
        if(!data.HaveSimpleValue< type >(#name)) \
            AddSimpleBranch< type >(#name, 0); \
        return *data.GetSimpleValue< type >(#name); \
    }


#define VECTOR_TREE_BRANCH(type, name) \
    std::vector< type >& name() \
    { \
        if(!data.HaveVectorValue< type >(#name)) \
            AddVectorBranch< type >(#name); \
        return *data.GetVectorValue< type >(#name); \
    }

namespace root_ext {
namespace detail {

    using std::string;

    template<typename DataType>
    struct DataAccessor;

    struct SmartTreeData {
        DEFINE_SMART_TREE_DATA_CONTAINER(Bool_t)
        DEFINE_SMART_TREE_DATA_CONTAINER(Int_t)
        DEFINE_SMART_TREE_DATA_CONTAINER(UInt_t)
        DEFINE_SMART_TREE_DATA_CONTAINER(Long64_t)
        DEFINE_SMART_TREE_DATA_CONTAINER(ULong64_t)
        DEFINE_SMART_TREE_DATA_CONTAINER(Double_t)
        DEFINE_SMART_TREE_DATA_CONTAINER(size_t)
        DEFINE_SMART_TREE_DATA_CONTAINER(Float_t)
        DEFINE_SMART_TREE_DATA_CONTAINER(string)

        ~SmartTreeData()
        {
            DELETE_SMART_TREE_DATA_CONTAINER(Bool_t)
            DELETE_SMART_TREE_DATA_CONTAINER(Int_t)
            DELETE_SMART_TREE_DATA_CONTAINER(UInt_t)
            DELETE_SMART_TREE_DATA_CONTAINER(Long64_t)
            DELETE_SMART_TREE_DATA_CONTAINER(ULong64_t)
            DELETE_SMART_TREE_DATA_CONTAINER(Double_t)
            DELETE_SMART_TREE_DATA_CONTAINER(size_t)
            DELETE_SMART_TREE_DATA_CONTAINER(Float_t)
            DELETE_SMART_TREE_DATA_CONTAINER(string)
        }

        template<typename DataType>
        bool HaveSimpleValue(const std::string& name) const
        {
            return DataAccessor<DataType>::HaveSimpleValue(*this, name);
        }

        template<typename DataType>
        DataType*& GetSimpleValue(const std::string& name)
        {
            return DataAccessor<DataType>::GetSimpleValue(*this, name);
        }

        template<typename DataType>
        bool HaveVectorValue(const std::string& name) const
        {
            return DataAccessor<DataType>::HaveVectorValue(*this, name);
        }

        template<typename DataType>
        std::vector<DataType>*& GetVectorValue(const std::string& name)
        {
            return DataAccessor<DataType>::GetVectorValue(*this, name);
        }

        void Reset()
        {
            CLEAR_SMART_TREE_DATA_CONTAINER(Bool_t)
            CLEAR_SMART_TREE_DATA_CONTAINER(Int_t)
            CLEAR_SMART_TREE_DATA_CONTAINER(UInt_t)
            CLEAR_SMART_TREE_DATA_CONTAINER(Long64_t)
            CLEAR_SMART_TREE_DATA_CONTAINER(ULong64_t)
            CLEAR_SMART_TREE_DATA_CONTAINER(Double_t)
            CLEAR_SMART_TREE_DATA_CONTAINER(size_t)
            CLEAR_SMART_TREE_DATA_CONTAINER(Float_t)
            CLEAR_SMART_TREE_DATA_CONTAINER(string)
        }
    };

    DEFINE_TREE_DATA_ACCESSOR(Bool_t)
    DEFINE_TREE_DATA_ACCESSOR(Int_t)
    DEFINE_TREE_DATA_ACCESSOR(UInt_t)
    DEFINE_TREE_DATA_ACCESSOR(Long64_t)
    DEFINE_TREE_DATA_ACCESSOR(ULong64_t)
    DEFINE_TREE_DATA_ACCESSOR(Double_t)
    DEFINE_TREE_DATA_ACCESSOR(size_t)
    DEFINE_TREE_DATA_ACCESSOR(Float_t)
    DEFINE_TREE_DATA_ACCESSOR(string)
} // detail

class SmartTree {
public:
    explicit SmartTree(const std::string& name, bool detachFromFile = true)
        : readMode(false)
    {
#ifdef SMART_TREE_FOR_CMSSW
        edm::Service<TFileService> tfserv;
        tree = tfserv->make<TTree>(name.c_str(), name.c_str());
#else
        tree = new TTree(name.c_str(), name.c_str());
#endif
        if(detachFromFile)
            tree->SetDirectory(0);
    }
    SmartTree(const std::string& name, TFile& file)
        : readMode(true)
    {
        tree = static_cast<TTree*>(file.Get(name.c_str()));
        if(!tree)
            throw std::runtime_error("Tree not found.");
        if(tree->GetNbranches())
            tree->SetBranchStatus("*", 0);
    }
    virtual ~SmartTree()
    {
#ifndef SMART_TREE_FOR_CMSSW
        if(!readMode)
            delete tree;
#endif
    }

    void Fill()
    {
        tree->Fill();
        data.Reset();
    }

    Long64_t GetEntries() const { return tree->GetEntries(); }
    void GetEntry(Long64_t entry) { tree->GetEntry(entry); }
    void Write() { tree->Write(); }

#ifndef SMART_TREE_FOR_CMSSW
    TTree& RootTree() { return *tree; }
#endif

protected:
    template<typename DataType>
    void AddSimpleBranch(const std::string& name, const DataType* default_value)
    {
        DataType*& value = data.GetSimpleValue<DataType>(name);
        const bool use_pointer = !default_value;
        value = use_pointer ? 0 : new DataType(*default_value);
        if(readMode) {
            tree->SetBranchStatus(name.c_str(), 1);
            if(use_pointer)
                tree->SetBranchAddress(name.c_str(), &value);
            else
                tree->SetBranchAddress(name.c_str(), value);
            if(tree->GetReadEntry() >= 0)
                tree->GetBranch(name.c_str())->GetEntry(tree->GetReadEntry());
        } else {
            TBranch* branch;
            if(use_pointer)
                branch = tree->Branch(name.c_str(), &value);
            else
                branch = tree->Branch(name.c_str(), value);
            const Long64_t n_entries = tree->GetEntries();
            for(Long64_t n = 0; n < n_entries; ++n)
                branch->Fill();
        }
    }

    template<typename DataType>
    void AddVectorBranch(const std::string& name)
    {
        std::vector<DataType>*& value = data.GetVectorValue<DataType>(name);
        value = new std::vector<DataType>();
        if(readMode) {
            tree->SetBranchStatus(name.c_str(), 1);
            tree->SetBranchAddress(name.c_str(), &value);
            if(tree->GetReadEntry() >= 0)
                tree->GetBranch(name.c_str())->GetEntry(tree->GetReadEntry());
        } else {
            TBranch* branch = tree->Branch(name.c_str(), value);
            const Long64_t n_entries = tree->GetEntries();
            for(Long64_t n = 0; n < n_entries; ++n)
                branch->Fill();
        }
    }

protected:
    detail::SmartTreeData data;

private:
    bool readMode;
    TTree* tree;
};
} // root_ext
