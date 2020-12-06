
#!/bin/bash
make all
mkdir times
timeNow=$(date +%d-%b-%H_%M_%S)  
echo "$timeNow"
touch "times/${timeNow}output.txt"

## declare an array variable that has the path of every mtx
declare -a mtx=("./matrices/belgium_osm/belgium_osm.mtx" "./matrices/com-Youtube/com-Youtube.mtx" "./matrices/dblp-2010/dblp-2010.mtx" "./matrices/mycielskian13/mycielskian13.mtx" "./matrixes/NACA0015/NACA0015.mtx")
declare -a programs=("./triangle_v3" "./triangle_v3_cilk" "./triangle_v3_opmp")



for program in "${programs[@]}"
do
    for matrix in "${mtx[@]}"
        do
            echo "$program" "$matrix" 1 2 >> "times/${timeNow}output.txt"
            "$program" "$matrix" 1 2 >> "times/${timeNow}output.txt"
            # Add a blank line between each iteration 
            echo >> "times/${timeNow}output.txt"
        done
        # Add a blank line between each iteration 
        echo >> "times/${timeNow}output.txt"    
done




