sudo lsof -i :4500 | awk 'NR > 1 { system("sudo kill -9 " $2) }'
sudo lsof -i :4501 | awk 'NR > 1 { system("sudo kill -9 " $2) }'
