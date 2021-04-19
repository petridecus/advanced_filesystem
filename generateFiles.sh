mkdir mnt/foo
for NUM in {1..5}
do
    echo $NUM > mnt/$NUM.txt
    cp mnt/$NUM.txt mnt/foo/$NUM.txt
done

rm mnt/5.txt

for NUM in {6..10}
do
    echo $NUM > mnt/$NUM.txt
    cp mnt/$NUM.txt mnt/foo/$NUM.txt
done

mv mnt/9.txt mnt/9a.txt

for NUM in {11..15}
do
    echo $NUM > mnt/$NUM.txt
    cp mnt/$NUM.txt mnt/foo/$NUM.txt
done

ln 13.txt 13b.txt
rm 13.txt


for NUM in {16..20}
do
    echo $NUM > mnt/$NUM.txt
    cp mnt/$NUM.txt mnt/foo/$NUM.txt
done
