clear
reset

set title "Simulation Spread Chart".ARG2
set key left Left
set offset -0.5

set term pdfcairo noenhanced font "Arial,11pt"
set output "spreadratio_4all_k"
#set terminal pdf noenhanced color font 'Helvetica,1'

set style fill solid
#set xtics rotate by -30 offset -3,0 nomirror
#set xtics font "Arial,12" 
set grid ytics

set ylabel	'Spread/#Nodes ratio'
set xlabel	'Twitter Sample Graphs (from SNAP)'

print ARG1


plot \
	"<awk '$2==1' ".ARG1  using (($5+$2)/$8):xtic(sprintf("%d Nodes",$8))  with histogram title "1 seed",\
	"<awk '$2==3' ".ARG1  using (($5+$2)/$8) with histogram title "3 seeds",\
	"<awk '$2==5' ".ARG1  using (($5+$2)/$8) with histogram title "5 seeds",\
	"<awk '$2==10' ".ARG1  using (($5+$2)/$8) with histogram title "10 seeds",\
	"<awk '$2==20' ".ARG1  using (($5+$2)/$8) with histogram title "20 seeds",\
	"<awk '$2==50' ".ARG1  using (($5+$2)/$8) with histogram title "50 seeds",\
	"<awk '$2==100' ".ARG1  using (($5+$2)/$8) with histogram title "100 seeds"
