// build.js
const esbuild = require('esbuild');
const nodeExternals = require('esbuild-node-externals').default; // <-- use .default

esbuild.build({
  entryPoints: ['mega.js'],
  bundle: true,
  outfile: 'MistBundle.js',
  platform: 'node',            // handles Node built-ins like 'net', 'url', etc.
  plugins: [nodeExternals()],  // include all node_modules automatically
  format: 'cjs',               // CommonJS for Node
}).catch(() => process.exit(1));
