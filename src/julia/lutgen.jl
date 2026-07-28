#!/usr/bin/env julia
# generate_allele_lut.jl
# Reads CYP2C19_allele_definition_table.xlsx and generates cyp2c19_allele_lut.h

using XLSX

# ─── Helpers ──────────────────────────────────────────────────────────

function safe_string(val)
    if val === nothing || ismissing(val)
        return ""
    end
    s = string(val)
    return strip(s)
end

function is_empty_or_blank(s)
    s = safe_string(s)
    return isempty(s) || all(isspace, s) || s == "N/A"
end

# Convert IUPAC nucleotide codes to regex pattern
function iupac_to_regex(char)
    char = string(char)
    map = Dict(
        'R' => "[AG]",
        'Y' => "[CT]",
        'S' => "[GC]",
        'W' => "[AT]",
        'K' => "[GT]",
        'M' => "[AC]",
        'B' => "[CGT]",
        'D' => "[AGT]",
        'H' => "[ACT]",
        'V' => "[ACG]",
        'N' => "[ACGT]",
    )
    return get(map, char, string(char))
end

# ─── Main ──────────────────────────────────────────────────────────────

function main()
    if length(ARGS) < 2
        println("Usage: julia lutgen.jl <input.xlsx> <output.h>")
        flush(stdout)
        exit(1)
    end

    input_file = ARGS[1]
    output_file = ARGS[2]

    println("Parsing allele definition table: $input_file")

    # ─── 1. Read the entire sheet as a 2D array ────────────────────

    data = XLSX.readdata(input_file, "Alleles", "A1:AJ46")
    rows = data
    first_col = rows[:, 1];

    # ─── 2. Find the row indices ────────────────────────────────────

    variant_row_idx = findfirst(rows -> safe_string(rows) == "Legacy Name/Class", first_col)
    if variant_row_idx === nothing
        error("Could not find 'Legacy Name/Class' row")
    end

    allele_row_idx = findfirst(rows -> safe_string(rows) == "CYP2C19 Allele", rows)
    if allele_row_idx === nothing
        error("Could not find 'CYP2C19 Allele' row")
    end

    # ─── 3. Extract the variant definitions ─────────────────────────

    variant_row = rows[variant_row_idx, :]
    protein_row = rows[variant_row_idx + 1, :]
    genomic_row = rows[variant_row_idx + 2, :]
    refseq_row = rows[variant_row_idx + 3, :]
    rsid_row = rows[variant_row_idx + 4, :]

    rsid_row = rows[6, 2:36]
    variant_id_of = Dict{Int, UInt64}()
    id::UInt64 = 0

    println("=== Variant ID Mapping (Column → ID) ===")
    println("Col | ID  | rsID")
    println("----|-----|-----------------")

    # damn off by 1 errors...

    for col_idx = 1:length(rsid_row)
        name = safe_string(variant_row[col_idx])
        if !is_empty_or_blank(name)
            variant_id_of[col_idx] = id
            rsid = safe_string(rsid_row[col_idx])
            println("$(lpad(col_idx, 2))  | $(lpad(id, 2))   | $rsid")
            id = id + 1
        end
    end
    println(variant_id_of)
    return variant_id_of

end

main()


