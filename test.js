const bstring = "01101100011011000110010101001000";
for (let i = 0; i < (bstring.length / 8); i++) {console.log(bstring.toString().slice(i*8, (i+1)*8))}
