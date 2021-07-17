mergeInto(LibraryManager.library, {
  mk_aud_buf: function (ptr, length, attributePtr, shdr2) {
    const flarr = new Float32Array(Module.HEAPF32, ptr, length);
    const zone = new Int16Array(Module.HEAP32, attributePtr, 60);
    const shdr = [
      ...new Uint32Array(Module.HEAPU32, shdr2 + 20, 5),
      ...new Uint8Array(Module.HEAPU8 + 40, 2),
    ];
    window.postMessage({ sample: { shdr, zone } });
  },
});
