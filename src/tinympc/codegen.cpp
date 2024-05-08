#include <stdio.h>
#include <ctype.h>
#include <time.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <iostream>
#include <Eigen/Dense>

// #include "types.hpp"
#include "codegen.hpp"

#ifdef __cplusplus
extern "C"
{
#endif

/* Define the maximum allowed length of the path (directory + filename + extension) */
#define PATH_LENGTH 2048

using namespace Eigen;

static void print_matrix(FILE *f, MatrixXd mat, int num_elements)
{
    for (int i = 0; i < num_elements; i++)
    {
        fprintf(f, "(tinytype)%.16f", mat.reshaped<RowMajor>()[i]);
        if (i < num_elements - 1)
            fprintf(f, ",");
    }
}


// Create inc/tiny_data.hpp file
int codegen_data_header(const char* output_dir, int verbose) {
    char data_hpp_fname[PATH_LENGTH];
    FILE *data_hpp_f;

    sprintf(data_hpp_fname, "%s/inc/tiny_data.hpp", output_dir);

    // Open source file
    data_hpp_f = fopen(data_hpp_fname, "w+");
    if (data_hpp_f == NULL)
        printf("ERROR OPENING TINY DATA HEADER FILE\n");

    // Preamble
    time_t start_time;
    time(&start_time);
    fprintf(data_hpp_f, "/*\n");
    fprintf(data_hpp_f, " * This file was autogenerated by TinyMPC on %s", ctime(&start_time));
    fprintf(data_hpp_f, " */\n\n");

    fprintf(data_hpp_f, "#pragma once\n\n");

    fprintf(data_hpp_f, "#include \"types.hpp\"\n\n");

    fprintf(data_hpp_f, "#ifdef __cplusplus\n");
    fprintf(data_hpp_f, "extern \"C\" {\n");
    fprintf(data_hpp_f, "#endif\n\n");

    fprintf(data_hpp_f, "extern TinySolver tiny_data_solver;\n\n");

    fprintf(data_hpp_f, "#ifdef __cplusplus\n");
    fprintf(data_hpp_f, "}\n");
    fprintf(data_hpp_f, "#endif\n");

    // Close codegen data header file
    fclose(data_hpp_f);

    if (verbose) {
        printf("Data header generated in %s\n", data_hpp_fname);
    }
    return 0;
}

// Create src/tiny_data.cpp file
int codegen_data_source(TinySolver* solver, const char* output_dir, int verbose) {
    char data_cpp_fname[PATH_LENGTH];
    FILE *data_cpp_f;

    int nx = solver->work->nx;
    int nu = solver->work->nu;
    int N = solver->work->N;

    sprintf(data_cpp_fname, "%s/src/tiny_data.cpp", output_dir);

    // Open source file
    data_cpp_f = fopen(data_cpp_fname, "w+");
    if (data_cpp_f == NULL)
        printf("ERROR OPENING TINY DATA SOURCE FILE\n");

    // Preamble
    time_t start_time;
    time(&start_time);
    fprintf(data_cpp_f, "/*\n");
    fprintf(data_cpp_f, " * This file was autogenerated by TinyMPC on %s", ctime(&start_time));
    fprintf(data_cpp_f, " */\n\n");

    // Open extern C
    fprintf(data_cpp_f, "#include \"tinympc/tiny_data.hpp\"\n\n");
    fprintf(data_cpp_f, "#ifdef __cplusplus\n");
    fprintf(data_cpp_f, "extern \"C\" {\n");
    fprintf(data_cpp_f, "#endif\n\n");

    // Solution
    fprintf(data_cpp_f, "/* Solution */\n");
    fprintf(data_cpp_f, "TinySettings settings = {\n");

    fprintf(data_cpp_f, "\t%d,\t\t// iter\n", solver->solution->iter);
    fprintf(data_cpp_f, "\t%d,\t\t// solved\n", solver->solution->solved);
    fprintf(data_cpp_f, "\t(tinyMatrix(%d, %d) << ", nx, N);
    print_matrix(data_cpp_f, MatrixXd::Zero(nx, N), nx * N);
    fprintf(data_cpp_f, ").finished(),\t// x\n"); // x solution
    fprintf(data_cpp_f, "\t(tinyMatrix(%d, %d) << ", nu, N-1);
    print_matrix(data_cpp_f, MatrixXd::Zero(nu, N-1), nu * N-1);
    fprintf(data_cpp_f, ").finished(),\t// x\n"); // u solution

    fprintf(data_cpp_f, "};\n\n");

    // Cache
    fprintf(data_cpp_f, "/* Matrices that must be recomputed with changes in time step, rho */\n");
    fprintf(data_cpp_f, "TinyCache cache = {\n");

    fprintf(data_cpp_f, "\t(tinytype)%.16f,\t// rho (step size/penalty)\n", solver->cache->rho);
    fprintf(data_cpp_f, "\t(tinyMatrix(%d, %d) << ", nu, nx);
    print_matrix(data_cpp_f, solver->cache->Kinf, nu * nx);
    fprintf(data_cpp_f, ").finished(),\t// Kinf\n"); // Kinf
    fprintf(data_cpp_f, "\t(tinyMatrix(%d, %d) << ", nx, nx);
    print_matrix(data_cpp_f, solver->cache->Pinf, nx * nx);
    fprintf(data_cpp_f, ").finished(),\t// Pinf\n"); // Pinf
    fprintf(data_cpp_f, "\t(tinyMatrix(%d, %d) << ", nu, nu);
    print_matrix(data_cpp_f, solver->cache->Quu_inv, nu * nu);
    fprintf(data_cpp_f, ").finished(),\t// Quu_inv\n"); // Quu_inv
    fprintf(data_cpp_f, "\t(tinyMatrix(%d, %d) << ", nx, nx);
    print_matrix(data_cpp_f, solver->cache->AmBKt, nx * nx);
    fprintf(data_cpp_f, ").finished(),\t// AmBKt\n"); // AmBKt

    fprintf(data_cpp_f, "};\n\n");

    // Settings
    fprintf(data_cpp_f, "/* User settings */\n");
    fprintf(data_cpp_f, "TinySettings settings = {\n");

    fprintf(data_cpp_f, "\t(tinytype)%.16f,\t// primal tolerance\n", solver->settings->abs_pri_tol);
    fprintf(data_cpp_f, "\t(tinytype)%.16f,\t// dual tolerance\n", solver->settings->abs_dua_tol);
    fprintf(data_cpp_f, "\t%d,\t\t// max iterations\n", solver->settings->max_iter);
    fprintf(data_cpp_f, "\t%d,\t\t// iterations per termination check\n", solver->settings->check_termination);
    fprintf(data_cpp_f, "\t%d,\t\t// enable state constraints\n", solver->settings->en_state_bound);
    fprintf(data_cpp_f, "\t%d\t\t// enable input constraints\n", solver->settings->en_input_bound);

    fprintf(data_cpp_f, "};\n\n");

    // Workspace
    fprintf(data_cpp_f, "/* Problem variables */\n");
    fprintf(data_cpp_f, "TinyWorkspace work = {\n");

    fprintf(data_cpp_f, "\t%d,\t// Number of states\n", nx);
    fprintf(data_cpp_f, "\t%d,\t// Number of control inputs\n", nu);
    fprintf(data_cpp_f, "\t%d,\t// Number of knotpoints in the horizon\n", N);

    fprintf(data_cpp_f, "\t(tinyMatrix(%d, %d) << ", nx, N);
    print_matrix(data_cpp_f, MatrixXd::Zero(nx, N), nx * N);
    fprintf(data_cpp_f, ").finished(),\t// x\n"); // x
    fprintf(data_cpp_f, "\t(tinyMatrix(%d, %d) << ", nu, N-1);
    print_matrix(data_cpp_f, MatrixXd::Zero(nu, N - 1), nu * (N - 1));
    fprintf(data_cpp_f, ").finished(),\t// u\n"); // u

    fprintf(data_cpp_f, "\t(tinyMatrix(%d, %d) << ", nx, N);
    print_matrix(data_cpp_f, MatrixXd::Zero(nx, N), nx * N);
    fprintf(data_cpp_f, ").finished(),\t// q\n"); // q
    fprintf(data_cpp_f, "\t(tinyMatrix(%d, %d) << ", nu, N-1);
    print_matrix(data_cpp_f, MatrixXd::Zero(nu, N - 1), nu * (N - 1));
    fprintf(data_cpp_f, ").finished(),\t// r\n"); // r

    fprintf(data_cpp_f, "\t(tinyMatrix(%d, %d) << ", nx, N);
    print_matrix(data_cpp_f, MatrixXd::Zero(nx, N), nx * N);
    fprintf(data_cpp_f, ").finished(),\t// p\n"); // p
    fprintf(data_cpp_f, "\t(tinyMatrix(%d, %d) << ", nu, N-1);
    print_matrix(data_cpp_f, MatrixXd::Zero(nu, N - 1), nu * (N - 1));
    fprintf(data_cpp_f, ").finished(),\t// d\n"); // d

    fprintf(data_cpp_f, "\t(tinyMatrix(%d, %d) << ", nx, N);
    print_matrix(data_cpp_f, MatrixXd::Zero(nx, N), nx * N);
    fprintf(data_cpp_f, ").finished(),\t// v\n"); // v
    fprintf(data_cpp_f, "\t(tinyMatrix(%d, %d) << ", nx, N);
    print_matrix(data_cpp_f, MatrixXd::Zero(nx, N), nx * N);
    fprintf(data_cpp_f, ").finished(),\t// vnew\n"); // vnew
    fprintf(data_cpp_f, "\t(tinyMatrix(%d, %d) << ", nu, N-1);
    print_matrix(data_cpp_f, MatrixXd::Zero(nu, N - 1), nu * (N - 1));
    fprintf(data_cpp_f, ").finished(),\t// z\n"); // z
    fprintf(data_cpp_f, "\t(tinyMatrix(%d, %d) << ", nu, N-1);
    print_matrix(data_cpp_f, MatrixXd::Zero(nu, N - 1), nu * (N - 1));
    fprintf(data_cpp_f, ").finished(),\t// znew\n"); // znew

    fprintf(data_cpp_f, "\t(tinyMatrix(%d, %d) << ", nx, N);
    print_matrix(data_cpp_f, MatrixXd::Zero(nx, N), nx * N);
    fprintf(data_cpp_f, ").finished(),\t// g\n"); // g
    fprintf(data_cpp_f, "\t(tinyMatrix(%d, %d) << ", nu, N-1);
    print_matrix(data_cpp_f, MatrixXd::Zero(nu, N - 1), nu * (N - 1));
    fprintf(data_cpp_f, ").finished(),\t// y\n"); // y

    fprintf(data_cpp_f, "\t(tinyVector(%d) << ", nx);
    print_matrix(data_cpp_f, solver->work->Q, nx);
    fprintf(data_cpp_f, ").finished(),\t// Q\n"); // Q
    fprintf(data_cpp_f, "\t(tinyVector(%d) << ", nu);
    print_matrix(data_cpp_f, solver->work->R, nu);
    fprintf(data_cpp_f, ").finished(),\t// R\n"); // R
    fprintf(data_cpp_f, "\t(tinyMatrix(%d, %d) << ", nx, nx);
    print_matrix(data_cpp_f, solver->work->Adyn, nx * nx);
    fprintf(data_cpp_f, ").finished(),\t// Adyn\n"); // Adyn
    fprintf(data_cpp_f, "\t(tinyMatrix(%d, %d) << ", nx, nu);
    print_matrix(data_cpp_f, solver->work->Bdyn, nx * nu);
    fprintf(data_cpp_f, ").finished(),\t// Bdyn\n"); // Bdyn

    fprintf(data_cpp_f, "\t(tinyMatrix(%d, %d) << ", nx, N);
    print_matrix(data_cpp_f, solver->work->x_min, nx * N);
    fprintf(data_cpp_f, ").finished(),\t// x_min\n"); // x_min
    fprintf(data_cpp_f, "\t(tinyMatrix(%d, %d) << ", nx, N);
    print_matrix(data_cpp_f, solver->work->x_max, nx * N);
    fprintf(data_cpp_f, ").finished(),\t// x_max\n"); // x_max
    fprintf(data_cpp_f, "\t(tinyMatrix(%d, %d) << ", nu, N-1);
    print_matrix(data_cpp_f, solver->work->u_min, nu * (N - 1));
    fprintf(data_cpp_f, ").finished(),\t// u_min\n"); // u_min
    fprintf(data_cpp_f, "\t(tinyMatrix(%d, %d) << ", nu, N-1);
    print_matrix(data_cpp_f, solver->work->u_max, nu * (N - 1));
    fprintf(data_cpp_f, ").finished(),\t// u_max\n"); // u_max
    
    fprintf(data_cpp_f, "\t(tinyMatrix(%d, %d) << ", nx, N);
    print_matrix(data_cpp_f, MatrixXd::Zero(nx, N), nx * N);
    fprintf(data_cpp_f, ").finished(),\t// Xref\n"); // Xref
    fprintf(data_cpp_f, "\t(tinyMatrix(%d, %d) << ", nu, N-1);
    print_matrix(data_cpp_f, MatrixXd::Zero(nu, N - 1), nu * (N - 1));
    fprintf(data_cpp_f, ").finished(),\t// Uref\n"); // Uref

    fprintf(data_cpp_f, "\t(tinyVector(%d) << ", nu);
    print_matrix(data_cpp_f, MatrixXd::Zero(nu, 1), nu);
    fprintf(data_cpp_f, ").finished(),\t// Qu\n"); // Qu

    fprintf(data_cpp_f, "\t(tinytype)%.16f,\t// state primal residual\n", 0.0);
    fprintf(data_cpp_f, "\t(tinytype)%.16f,\t// input primal residual\n", 0.0);
    fprintf(data_cpp_f, "\t(tinytype)%.16f,\t// state dual residual\n", 0.0);
    fprintf(data_cpp_f, "\t(tinytype)%.16f,\t// input dual residual\n", 0.0);
    fprintf(data_cpp_f, "\t%d,\t// solve status\n", 0);
    fprintf(data_cpp_f, "\t%d,\t// solve iteration\n", 0);
    fprintf(data_cpp_f, "};\n\n");

    // Write solver struct definition to workspace file
    fprintf(data_cpp_f, "TinySolver tiny_data_solver = {&solution, &settings, &cache, &work};\n\n");

    // Close extern C
    fprintf(data_cpp_f, "#ifdef __cplusplus\n");
    fprintf(data_cpp_f, "}\n");
    fprintf(data_cpp_f, "#endif\n\n");

    // Close codegen data file
    fclose(data_cpp_f);
    if (verbose) {
        printf("Data generated in %s\n", data_cpp_fname);
    }
    return 0;
}

int codegen_example(const char* output_dir, int verbose) {
    char example_cpp_fname[PATH_LENGTH];
    FILE *example_cpp_f;

    sprintf(example_cpp_fname, "%s/src/tiny_main.cpp", output_dir);

    // Open global options file
    example_cpp_f = fopen(example_cpp_fname, "w+");
    if (example_cpp_f == NULL)
        printf("ERROR OPENING EXAMPLE MAIN FILE\n");

    // Preamble
    time_t start_time;
    time(&start_time);
    fprintf(example_cpp_f, "/*\n");
    fprintf(example_cpp_f, " * This file was autogenerated by TinyMPC on %s", ctime(&start_time));
    fprintf(example_cpp_f, " */\n\n");

    fprintf(example_cpp_f, "#include <iostream>\n\n");

    fprintf(example_cpp_f, "#include <tinympc/tiny_api.hpp>\n");
    fprintf(example_cpp_f, "#include <tinympc/tiny_data.hpp>\n\n");

    fprintf(example_cpp_f, "using namespace Eigen;\n");
    fprintf(example_cpp_f, "IOFormat TinyFmt(4, 0, \", \", \"\\n\", \"[\", \"]\");\n\n");

    fprintf(example_cpp_f, "#ifdef __cplusplus\n");
    fprintf(example_cpp_f, "extern \"C\" {\n");
    fprintf(example_cpp_f, "#endif\n\n");

    fprintf(example_cpp_f, "int main()\n");
    fprintf(example_cpp_f, "{\n");
    fprintf(example_cpp_f, "\tint exitflag = 1;\n");
    fprintf(example_cpp_f, "\t// Double check some data\n");
    fprintf(example_cpp_f, "\tstd::cout << tiny_data_solver.settings->max_iter << std::endl;\n");
    fprintf(example_cpp_f, "\tstd::cout << tiny_data_solver.cache->AmBKt.format(TinyFmt) << std::endl;\n");
    fprintf(example_cpp_f, "\tstd::cout << tiny_data_solver.work->Adyn.format(TinyFmt) << std::endl;\n\n");

    fprintf(example_cpp_f, "\t// Visit https://tinympc.org/ to see how to set the initial condition and update the reference trajectory.\n\n");

    fprintf(example_cpp_f, "\texitflag = tiny_solve(&tiny_data_solver);\n\n");
    fprintf(example_cpp_f, "\tif (exitflag == 0) printf(\"Hooray! Solved with no error!\\n\");\n");
    fprintf(example_cpp_f, "\telse printf(\"Oops! Something went wrong!\\n\");\n");

    fprintf(example_cpp_f, "\treturn 0;\n");
    fprintf(example_cpp_f, "}\n\n");

    fprintf(example_cpp_f, "#ifdef __cplusplus\n");
    fprintf(example_cpp_f, "} /* extern \"C\" */\n");
    fprintf(example_cpp_f, "#endif\n");

    // Close codegen example main file
    fclose(example_cpp_f);
    if (verbose) {
        printf("Example tinympc main generated in %s\n", example_cpp_fname);
    }
    return 0;
}

#ifdef __cplusplus
}
#endif