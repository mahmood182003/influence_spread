clear
reset

set title "Influence Spread, comparing all 3 seed selection methods, k=".ARG1
set key left Left
set offset -0.5

set term pdfcairo noenhanced font "Arial,11pt"
set output "spreadratio_cross_k".ARG1
#set terminal pdf noenhanced color font 'Helvetica,1'

set style fill solid
#set xtics rotate by -30 offset -3,0 nomirror
#set xtics font "Arial,12" 
set grid ytics

set ylabel	'Spread/#Nodes ratio'
set xlabel	'Twitter Sample Graphs (from SNAP)'

print ARG1
print ARG2
print ARG3


plot \
	"<awk '$2=='".ARG1." result_greedy.txt"  using (($5+$2)/$8):xtic(sprintf("%d Nodes",$8))  with histogram title "Greedy",\
	"<awk '$2=='".ARG1." result_hd.txt"  using (($5+$2)/$8) with histogram title "High-Degree",\
	"<awk '$2=='".ARG1." result_pr.txt"  using (($5+$2)/$8) with histogram title "Page-Rank"