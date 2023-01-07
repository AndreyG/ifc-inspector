#pragma once

#include "schema.h"

class Commander
{
    Schema schema_;
    ifc::File const& file_;
    std::unordered_map<std::string_view, ifc::PartitionSummary const*> partition_summary_;

    using ArgumentList = std::span<const std::string_view>;

    std::unordered_map<std::string_view, void(Commander::*)(ArgumentList)> commands_ = {
        { "partition", &Commander::partition },
        { "element",   &Commander::element },
    };

private:
    void present_partitions();
    size_t calculate_size_of(PartitionDescription const&) const;
    size_t size_of(ifc::TypeIndex) const;
    size_t size_of(ifc::FundamentalType) const;
    size_t size_of(ifc::ScopeDeclaration const&) const;
    void present_partition(std::string_view partition);
    void partition(ArgumentList arguments);
    void element(ArgumentList arguments);
    void present_element(std::string_view partition, uint32_t index, ifc::PartitionSummary const&);
    void present_element(std::byte const* data_ptr, PartitionDescription const&);
    void present_value(std::byte const*& data_ptr, ifc::TypeIndex);
    void present_object_value(const std::byte*& data_ptr, ifc::ScopeDeclaration const&);
    void present_enumerator(std::byte const*& data_ptr, ifc::Enumeration const&);
    void present_abstract_reference(uint32_t storage, ifc::ExprIndex first_templarg, ifc::ExprIndex second_templarg);

    void present_partition_description(PartitionDescription const& partition);
    void present_type(ifc::TypeIndex);
    void present_fundamental_type(ifc::FundamentalType);

    std::uint32_t get_int_literal_value(ifc::ExprIndex) const;

public:
    Commander(ifc::File::BlobView schema_raw_data, ifc::File const& file);

    void operator() (std::string_view command);
};
