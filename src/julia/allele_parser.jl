module allele_parser

using XLSX

struct variant
    name::String
    protein_effect::String
    genomic_pos::String
    refseq_pos::String
    rsid::String
    bitmask_index::Int
end

struct star_allele
    name::String
    mask::UInt64
    protein_effect_index::Int
end

function parse_allele_string(filepath::String)

    xf = XLSX.readxlsx(filepath)
    sheet = xf["Alleles"]

    data = XLSX.gettable(sheet)

    
