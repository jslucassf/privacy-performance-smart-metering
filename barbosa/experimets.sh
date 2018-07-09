for i in `seq 10 10 200`;
do 
  echo "Generating samples with N = $i..."
  for j in `seq 1 10`;
  do
    ./barbosa_simulator $i >> $i.txt;
  done
done
