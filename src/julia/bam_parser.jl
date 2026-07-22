module bam_parser

using XAM

reader = open(BAM.Reader, "../data/C26B265CYP2C19_L00.bam")

for record in reader
	if BAM.ismapped(record)
		println(BAM.refname(record), ':', BAM.position(record))
	end 
end

close(reader)

end
