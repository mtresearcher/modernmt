//
// Created by Davide  Caroselli on 23/08/16.
//

#ifndef FASTALIGN_BUILDER_H
#define FASTALIGN_BUILDER_H

#include <string>
#include <vector>
#include "Model.h"
#include "Corpus.h"

namespace mmt {
    namespace fastalign {

        struct Options {
            double mean_srclen_multiplier = 1.0;
            int iterations = 5;
            bool favor_diagonal = true;
            double prob_align_null = 0.08;
            double initial_diagonal_tension = 4.0;
            bool optimize_tension = true;
            bool variational_bayes = true;
            double alpha = 0.01;
            bool use_null = true;
            int threads = 0; // Default is number of CPUs
            size_t buffer_size = 10000;
        };

        typedef int BuilderStep;

        static const BuilderStep kBuilderStepSetup = 1;
        static const BuilderStep kBuilderStepAligning = 2;
        static const BuilderStep kBuilderStepOptimizingDiagonalTension = 3;
        static const BuilderStep kBuilderStepNormalizing = 4;
        static const BuilderStep kBuilderStepPruning = 5;

        class Builder {
        public:

            class Listener {
            public:
                virtual void Begin(bool forward) = 0;

                virtual void IterationBegin(bool forward, int iteration) = 0;

                virtual void Begin(bool forward, const BuilderStep step, int iteration) = 0;

                virtual void End(bool forward, const BuilderStep step, int iteration) = 0;

                virtual void IterationEnd(bool forward, int iteration) = 0;

                virtual void End(bool forward) = 0;

                virtual void ModelDumpBegin() = 0;

                virtual void ModelDumpEnd() = 0;
            };

            Builder(Options options = Options());

            void setListener(Listener *listener);

            void Build(const Corpus &corpus, const std::string &path);

        private:
            const double mean_srclen_multiplier;
            const double initial_diagonal_tension;
            const int iterations;
            const bool favor_diagonal;
            const double prob_align_null;
            const bool optimize_tension;
            const bool variational_bayes;
            const double alpha;
            const bool use_null;
            const size_t buffer_size;
            const int threads;

            Listener *listener;

            void AllocateTTableSpace(Model *_model, const std::unordered_map<wid_t, std::vector<wid_t>> &values,
                                     const wid_t sourceWordMaxValue);

            void InitialPass(const Corpus &corpus, double *n_target_tokens, Model *model,
                             std::vector<std::pair<std::pair<length_t, length_t>, size_t>> *size_counts);

            Model *BuildModel(const Corpus &corpus, bool forward);

        };

    }
}

#endif //FASTALIGN_BUILDER_H