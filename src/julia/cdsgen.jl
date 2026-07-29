#!/usr/bin/env julia
# cdsgen.jl
# Reads CYP2C19_CDS.xlsx and generates cyp2c19_cds_lut.h

using XLSX, Dates

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

# ─── Main ──────────────────────────────────────────────────────────────

# This sheet is very simple and doesn't require much complexity in the way of LUTs
# 8 rows, with the EHR notation column basically consisting of 3 possible values:
#   > None;
#   > Abnormal/Priority/High Risk
#   > Normal/Routine/Low Risk

function main()
    if length(ARGS) < 2
        println("Usage: julia lutgen.jl <input.xlsx> <output.h>")
        flush(stdout)
        exit(1)
    end

    input_file = ARGS[1]
    output_file = ARGS[2]

    println("Parsing CDS reference table: $input_file")

# ─── 1. Read the entire sheet as a 2D array ────────────────────

    data = XLSX.readdata(input_file, "CDS", "A1:D10")
    rows = data
    header_row = rows[2, :]

# ─── 2. Find the column indices ────────────────────────────────────

    pheno_col_idx = findfirst(c -> safe_string(c) == "CYP2C19 Phenotype", header_row)
    if pheno_col_idx === nothing
        error("Could not find 'CYP2C19 Phenotype' column")
    end

    ehr_col_idx = findfirst(c -> safe_string(c) == "EHR Priority Result Notation", header_row)
    if ehr_col_idx === nothing
        error("Could not find 'EHR Priority Result Notation' column")
    end

    consult_col_idx = findfirst(c -> safe_string(c) == "Consultation (Interpretation) Text Provided with Test Result", header_row)
    if consult_col_idx === nothing
        error("Could not find 'Consultation (Interpretation) Text Provided with Test Result' column")
    end

# ─── 3. Extract the phenotype rows ─────────────────────────────

    phenotypes = []
    for row_idx in 3:size(rows, 1)  # Data starts at row 3
        row = rows[row_idx, :]
        pheno_name = safe_string(row[pheno_col_idx])
        if is_empty_or_blank(pheno_name)
            continue
        end

        priority = safe_string(row[ehr_col_idx])
        consultation = safe_string(row[consult_col_idx])

        push!(phenotypes, (
            name = pheno_name,
            priority = priority,
            consultation = consultation
            ))
    end

#     println("  Found $(length(phenotypes)) phenotypes.")

# ─── 4. Print the header ──────────────────────────────────────

phenotype_names = []
ehr_priority = []
notation_text = []

    for p in phenotypes

        push!(phenotype_names, p.name)
        push!(ehr_priority, p.priority)
        push!(notation_text, p.consultation)

    end
                    open("../../includes/cyp2c19_cds_lut.h", "w") do io
                    println(io, "/* Header automatically generated at $(now(UTC)) */")
                    println(io, "#ifndef CYP2C19_CDS_LUT_H")
                    println(io, "#define CYP2C19_CDS_LUT_H")

                    println(io, "static const char * const cyp2c19_phenotype_names[] = {$phenotype_names};")
                    println(io, "static const char * const cyp2c19_ehr_notation[] = {$ehr_priority};")
                    println(io, "static const char * const cyp2c19_consultation_text[] = {$notation_text};")

                    println(io, "#endif")
        end
end

main()
