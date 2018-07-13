function writeColNames {
    declare -a experiments=("garcia-20-1024", "garcia-20-2048", "garcia-50-1024", "garcia-50-2048",
                            "erkin-20-1024", "erkin-20-2048", "erkin-50-1024", "erkin-50-2048");
    
    echo -n "experimento" >> experiments_adsd.csv;
    echo -n "," >> experiments_adsd.csv;

    for i in "${experiments[@]}"
    do
        echo -n "$i" >> experiments_adsd.csv;
        #if [ $i != "erkin-20-2048" ]
        #then
        #    echo -n "," >> experiments_adsd.csv;
        #fi
    done

    echo >> experiments_adsd.csv;
}

function execExperiments {
    for j in `seq 1 20`;
    do
        echo "$j";
        echo -n "$j" >> experiments_adsd.csv;
        echo -n "," >> experiments_adsd.csv;

        ./garcia/adsd/g_1024 20 -n >> experiments_adsd.csv;
        echo -n "," >> experiments_adsd.csv;

        ./garcia/adsd/g_2048 20 -n >> experiments_adsd.csv;
        echo -n "," >> experiments_adsd.csv;

        ./garcia/adsd/g_1024 50 -n >> experiments_adsd.csv;
        echo -n "," >> experiments_adsd.csv;
        
        ./garcia/adsd/g_2048 50 -n >> experiments_adsd.csv;
        echo -n "," >> experiments_adsd.csv;

        ./erkin/adsd/e_1024 20 -n >> experiments_adsd.csv;
        echo -n "," >> experiments_adsd.csv;

        ./erkin/adsd/e_2048 20 -n >> experiments_adsd.csv;
        echo -n "," >> experiments_adsd.csv;

        ./erkin/adsd/e_1024 50 -n >> experiments_adsd.csv;
        echo -n "," >> experiments_adsd.csv;
        
        ./erkin/adsd/e_2048 50 -n >> experiments_adsd.csv;
        
        echo >> experiments_adsd.csv;

    done
}

writeColNames
execExperiments