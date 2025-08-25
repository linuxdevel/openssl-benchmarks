# Makefile for RSA, EC Key Generators and ECDSA Signer

# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra -O2 -pthread
LDFLAGS = -lssl -lcrypto -pthread

# Directories
SRCDIR = src/cpp
OBJDIR = obj

# Target executables
RSA_TARGET = rsa_generator
EC_TARGET = ec_generator
ECDSA_TARGET = ecdsa_signer
BENCHMARK_TARGET = crypto_benchmark

# Source files
RSA_SOURCES = $(SRCDIR)/rsa_generator.cpp
EC_SOURCES = $(SRCDIR)/ec_generator.cpp
ECDSA_SOURCES = $(SRCDIR)/ecdsa_signer.cpp
BENCHMARK_SOURCES = $(SRCDIR)/crypto_benchmark.cpp

# Object files
RSA_OBJECTS = $(OBJDIR)/rsa_generator.o
EC_OBJECTS = $(OBJDIR)/ec_generator.o
ECDSA_OBJECTS = $(OBJDIR)/ecdsa_signer.o
BENCHMARK_OBJECTS = $(OBJDIR)/crypto_benchmark.o

# Default target - build all generators
all: $(OBJDIR) $(RSA_TARGET) $(EC_TARGET) $(ECDSA_TARGET) $(BENCHMARK_TARGET)

# Create object directory
$(OBJDIR):
	mkdir -p $(OBJDIR)

# Build the RSA generator
$(RSA_TARGET): $(RSA_OBJECTS)
	$(CXX) $(RSA_OBJECTS) -o $(RSA_TARGET) $(LDFLAGS)

# Build the EC generator
$(EC_TARGET): $(EC_OBJECTS)
	$(CXX) $(EC_OBJECTS) -o $(EC_TARGET) $(LDFLAGS)

# Build the ECDSA signer
$(ECDSA_TARGET): $(ECDSA_OBJECTS)
	$(CXX) $(ECDSA_OBJECTS) -o $(ECDSA_TARGET) $(LDFLAGS)

# Build the crypto benchmark
$(BENCHMARK_TARGET): $(BENCHMARK_OBJECTS)
	$(CXX) $(BENCHMARK_OBJECTS) -o $(BENCHMARK_TARGET) $(LDFLAGS) -lm

# Build object files
$(OBJDIR)/%.o: $(SRCDIR)/%.cpp | $(OBJDIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean build artifacts
clean:
	rm -rf $(OBJDIR) $(RSA_TARGET) $(EC_TARGET) $(ECDSA_TARGET) $(BENCHMARK_TARGET)

# Install dependencies (Ubuntu/Debian)
install-deps:
	sudo apt-get update
	sudo apt-get install -y build-essential libssl-dev

# Test run with default parameters for all tools
test: $(RSA_TARGET) $(EC_TARGET) $(ECDSA_TARGET) $(BENCHMARK_TARGET)
	@echo "Testing RSA generator:"
	./$(RSA_TARGET) 2048 2 10
	@echo ""
	@echo "Testing EC generator:"
	./$(EC_TARGET) P256 2 20
	@echo ""
	@echo "Testing ECDSA signer:"
	./$(ECDSA_TARGET) P256 2 100
	@echo ""
	@echo "Testing crypto benchmark:"
	./$(BENCHMARK_TARGET)

# Test EC key generation with different curves
test-ec: $(EC_TARGET)
	@echo "Testing P-256 curve:"
	./$(EC_TARGET) P256 4 25
	@echo ""
	@echo "Testing P-384 curve:"
	./$(EC_TARGET) P384 4 15
	@echo ""
	@echo "Testing P-521 curve:"
	./$(EC_TARGET) P521 4 10

# Test ECDSA signing with different curves
test-ecdsa: $(ECDSA_TARGET)
	@echo "Testing ECDSA P-256 signing:"
	./$(ECDSA_TARGET) P256 4 250
	@echo ""
	@echo "Testing ECDSA P-384 signing:"
	./$(ECDSA_TARGET) P384 4 200
	@echo ""
	@echo "Testing ECDSA P-521 signing:"
	./$(ECDSA_TARGET) P521 4 150

# Help target
help:
	@echo "Available targets:"
	@echo "  all           - Build all generators, signers and benchmark (default)"
	@echo "  rsa_generator - Build only the RSA generator"
	@echo "  ec_generator  - Build only the EC generator"
	@echo "  ecdsa_signer  - Build only the ECDSA signer"
	@echo "  crypto_benchmark - Build only the crypto benchmark"
	@echo "  clean         - Remove build artifacts"
	@echo "  install-deps  - Install required dependencies (Ubuntu/Debian)"
	@echo "  test          - Build and run tests for all tools"
	@echo "  test-ec       - Build and run EC generator tests with different curves"
	@echo "  test-ecdsa    - Build and run ECDSA signer tests with different curves"
	@echo "  help          - Show this help message"
	@echo ""
	@echo "Usage after building:"
	@echo "  ./$(RSA_TARGET) <keysize> <num_threads> <num_loops>"
	@echo "  ./$(EC_TARGET) <curve> <num_threads> <num_loops>"
	@echo "  ./$(ECDSA_TARGET) <curve> <num_threads> <num_loops>"
	@echo "  ./$(BENCHMARK_TARGET)  # No parameters needed"
	@echo ""
	@echo "Examples:"
	@echo "  ./$(RSA_TARGET) 2048 4 100     # RSA 2048-bit keys"
	@echo "  ./$(EC_TARGET) P256 4 100      # EC P-256 keys"
	@echo "  ./$(ECDSA_TARGET) P256 4 1000  # ECDSA P-256 signatures"
	@echo "  ./$(BENCHMARK_TARGET)          # RSA vs ECDSA performance comparison"
	@echo "  ./$(EC_TARGET) --curves        # List supported EC curves"

.PHONY: all clean install-deps test test-ec test-ecdsa help
