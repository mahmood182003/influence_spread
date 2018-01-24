clear
reset

set title ARG2
set key left Left outside
set offset -0.5

set term pdfcairo# noenhanced font "Arial,11pt"
set output "spreadratio_greedy"
#set terminal pdf noenhanced color font 'Helvetica,1'

set boxwidth 0.8
set style fill solid
set xtics out nomirror #rotate by -30 offset -3,0 nomirror
#set xtics font "Arial,12" 
set grid ytics

unset bars


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
	"<awk '$2==100' ".ARG1  using (($5+$2)/$8) with histogram title "100 seeds",\
	"<awk '$2==1' ".ARG1  using ($0-0.28):(($4+$2)/$8):(0.05) with xerrorbars ls -1 lc black title "Expected Spread",\
	"<awk '$2==3' ".ARG1  using ($0-0.17):(($4+$2)/$8):(0.05) with xerrorbars ls -1 lc black notitle,\
	"<awk '$2==5' ".ARG1  using ($0-0.055):(($4+$2)/$8):(0.05) with xerrorbars ls -1 lc black notitle,\
	"<awk '$2==10' ".ARG1  using ($0+0.055):(($4+$2)/$8):(0.05) with xerrorbars ls -1 lc black notitle,\
	"<awk '$2==20' ".ARG1  using ($0+0.17):(($4+$2)/$8):(0.05) with xerrorbars ls -1 lc black notitle,\
	"<awk '$2==50' ".ARG1  using ($0+0.28):(($4+$2)/$8):(0.05) with xerrorbars ls -1 lc black notitle,\
	"<awk '$2==100' ".ARG1  using ($0+0.39):(($4+$2)/$8):(0.05) with xerrorbars ls -1 lc black notitle
		
	
	