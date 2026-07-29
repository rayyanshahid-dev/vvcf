#!/usr/bin/env julia
# lutgen.jl
# Reads CYP2C19_allele_definition_table.xlsx and generates cyp2c19_allele_lut.h

using XLSX, StructArrays, CSV, Dates

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

# TODO --> see below, we should handle these properly in relation to the bitmasks
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
    first_row = rows[2, :];
    last_row  = size(rows, 1)

    # ─── 2. Find the row indices ────────────────────────────────────

    variant_row_idx = findfirst(rows -> safe_string(rows) == "Legacy Name/Class", first_col)
    if variant_row_idx === nothing
        error("Could not find 'Legacy Name/Class' row")
    end

    allele_row_idx = findfirst(rows -> safe_string(rows) == "CYP2C19 Allele", first_col)
    if allele_row_idx === nothing
        error("Could not find 'CYP2C19 Allele' row")
    end

    # ─── 3. Extract the safe_string variant definitions ─────────────────────────

    variant_row = rows[variant_row_idx, :]
    protein_row = rows[variant_row_idx + 1, :]
    genomic_row = rows[variant_row_idx + 2, :]
    refseq_row = rows[variant_row_idx + 3, :]
    rsid_row = rows[variant_row_idx + 4, :] # better not to hardcode these...

    variant_id_of = Dict{Int, UInt64}()
    id::UInt64 = 0

    println("\n=== Variant ID Mapping (Column → ID) ===\n")
    println("Col | ID  | rsID")
    println("----|-----|-----------------")


    for col_idx = 2:length(rsid_row) # 1-index vs 0-index
        name = safe_string(variant_row[col_idx])
        if !is_empty_or_blank(name)
            variant_id_of[col_idx] = id
            rsid = safe_string(rsid_row[col_idx])
           println("$(lpad(col_idx, 2))  | $(lpad(id, 2)) | $rsid")
            id = id + 1
        end
    end
    println("rsID_count | Mask")
    println("-----------|-----")
        for (col, var_id) in sort(collect(variant_id_of))
            println("$col => 0x$(string(var_id, base=16, pad=16)),")
        end
    println(")")

# ─── 4. Allele mask generation ─────────────────────────

# Get the integer row indices
first_allele_row = allele_row_idx[1] + 1
last_allele_row = size(rows, 1)

allele_names = []
allele_masks = []

for row_idx in first_allele_row:last_allele_row
    row = rows[row_idx, :]
    name = safe_string(row[1])

    if is_empty_or_blank(name)
        continue
    end

    if name == "*38"
        continue
    end

    mask::UInt64 = 0

    for col_idx in 2:length(row)
        cell = safe_string(row[col_idx])

        if is_empty_or_blank(cell)
            continue
        end

        if !(cell in ("A", "C", "G", "T")) # TODO properly handle the IUPAC ambiguity codes
            continue
        end

        variant_id = variant_id_of[col_idx] # this is offset by one already, so make sure not to do [col_idx - 1] to compensate
        mask |= UInt64(1) << variant_id
    end

    push!(allele_names, name)
    push!(allele_masks, mask)
end

# ─── 5. Print the results to stdout ──────────────────────────────

println("\n=== Allele Masks ===")
println("Allele | Mask")
println("-------|-----")
for (name, mask) in zip(allele_names, allele_masks)
    println("$(rpad(name, 6)) | 0x$(string(mask, base=16, pad=16))")
end

println("\nGenerated $(length(allele_names)) allele masks.")

# ─── 6. Revised allele matching function ──────────────────────────────

# this was a little tricky and hinges on making sure we don't have *42, *36, or *37 appearing
# where they're not supposed to, since their masks are all 0x0 and would show up if naively resolved
function allele_match(observed_mask::UInt, allele_names::Vector{String}, allele_masks::Vector{UInt64})
    candidates = [];
    NO_MATCH = 0x01
    AMBIGUOUS = 0x02

    for i in 1:length(allele_masks)
        if allele_masks[i] == 0
            continue
        end

        if (observed_mask & allele_masks[i]) == allele_masks[i]
            push!(candidates, (allele_names[i], allele_masks[i]))
        end
    end

       if isempty(candidates)
            return NO_MATCH
        end

        best_popcount = maximum(count_ones(mask) for (name, mask) in candidates)
            best = [name for (name, mask) in candidates if count_ones(mask) == best_popcount]

                if length(best) == 1
                    return best[1]
                else
                    return AMBIGUOUS
                end
        end

            open("../../includes/cyp2c19_allele_lut.h", "w") do io
                println(io, "/* Header automatically generated at $(now(UTC)) */")
                println(io, "#ifndef CYP2C19_ALLELE_LUT_H")
                println(io, "#define CYP2C19_ALLELE_LUT_H")

                println(io, "static const char * const cyp2c19_allele_names[] = {$allele_names};")
                println(io, "static const uint64_t cyp2c19_allele_masks[] = {$allele_masks};")
                println(io, "#define CYP2C19_ALLELE_COUNT $(length(allele_names))")
                println(io, "#endif")
            end
end
main()
