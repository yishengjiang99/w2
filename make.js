const fs = require("fs");

const execSync = require("child_process").execSync;

execSync("npx wa compile src/pdta.c -o build/pdta.wasm");

fs.writeFileSync(
  `build/pdta.js`,
  /* javascript */ `// prettier-ignore
  const wasmBinary = new Uint8Array([
    ${fs.readFileSync("build/pdta.wasm").join(",")}
  ]);   
   const mem = new WebAssembly.Memory({
    initial: 100, //100 x 64k ..just putting in some safe values now
    maximum: 100,
  });
 async  function init(){
   const r= await WebAssembly.instantiate(wasmBinary, {
      env: {
        memory: mem,
        consolelog:(a,b)=>console.log(new Uint8Array(mem.buffer,a,b).map(c=>String.fromCharCode(c)).join('')),
        table: new WebAssembly.Table({ element: "anyfunc", initial: 6 }),
        _abort:()=>console.log('ab'),
        _grow:(a,b)=>console.log(a,b),
        memcpy:(a,b,c)=>console.log(a,b,c)
      },
    });
    return {
        ...r.instance.exports,mem
    }

  }
  export {init,mem};
  
  `
);
