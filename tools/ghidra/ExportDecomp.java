// Exports every function of the open program as decompiled C, one .c file per
// module, plus a function index. Run by ghidra.cmd through analyzeHeadless.
// Argument 1: output folder.
import java.io.BufferedWriter;
import java.io.File;
import java.io.FileWriter;
import java.io.PrintWriter;

import ghidra.app.decompiler.DecompInterface;
import ghidra.app.decompiler.DecompileResults;
import ghidra.app.script.GhidraScript;
import ghidra.program.model.listing.Function;
import ghidra.program.model.listing.FunctionIterator;

public class ExportDecomp extends GhidraScript {
    @Override
    protected void run() throws Exception {
        String[] args = getScriptArgs();
        File dir = new File(args.length > 0 ? args[0] : ".");
        dir.mkdirs();
        String name = currentProgram.getName().replaceAll("\\.[^.]*$", "");

        DecompInterface decomp = new DecompInterface();
        decomp.openProgram(currentProgram);

        int done = 0, failed = 0;
        try (PrintWriter c = new PrintWriter(new BufferedWriter(new FileWriter(new File(dir, name + ".c"))));
             PrintWriter idx = new PrintWriter(new BufferedWriter(new FileWriter(new File(dir, name + ".idx.txt"))))) {
            FunctionIterator it = currentProgram.getFunctionManager().getFunctions(true);
            while (it.hasNext() && !monitor.isCancelled()) {
                Function f = it.next();
                DecompileResults r = decomp.decompileFunction(f, 60, monitor);
                c.println("// ==== " + f.getEntryPoint() + " " + f.getName() + " ====");
                if (r.decompileCompleted()) {
                    c.println(r.getDecompiledFunction().getC());
                } else {
                    c.println("// decompile failed: " + r.getErrorMessage());
                    failed++;
                }
                idx.println(f.getEntryPoint() + "\t" + f.getBody().getNumAddresses() + "\t" + f.getName());
                if (++done % 500 == 0)
                    println(name + ": " + done + " functions");
            }
        }
        println(name + ": " + done + " functions exported, " + failed + " failed");
    }
}
