
#include"Differential_Evolution.h"

// random 	The random variable should be initialized before
// F		Should be between [0,2]	
// CR		Should be between [0,1]	
Differential_Evolution::Differential_Evolution(int max_generations, int population_size, double max_limit, double min_limit, float F, float CR, Random* random)
{
	generation=1;
	this->max_generations= max_generations;
	
	this->population_size= population_size;

	this->max_limit= max_limit;
	this->min_limit= min_limit;

	this->random=random;

	fitness=(double*)malloc(sizeof(double)*population_size);
	population= (double**)malloc(sizeof(double*)*population_size);

	this->F= F;
	this->CR= CR;
	
	//if the problem size was already determined 
	//(this is usually defined when the problem is defined)
	problem_previously_defined=false;

	//variables for continuying evolution later on
	keep_previous_individuals=false;
}

Differential_Evolution::~Differential_Evolution()
{
	int i;

	//if existent, free all problem related memory
	if(problem_previously_defined==true)
	{
		freeProblemMemory();
	}

	//free the remaining population related memory
	free(population);
	free(fitness);
		
}

//Makes the DE reuse the individuals from a previous evolution
//initializing only the additional dna spectrum
void Differential_Evolution::keepPreviousIndividuals()
{
	keep_previous_individuals=true;
}

void Differential_Evolution::mutationOperator(int individual)
{
	int i;

	if(population_size >= 4)
	{
		int r1, r2, r3; //index of individuals

		r1= random->uniform(0,population_size-1);

		while(r1 == individual)
		{
			r1= random->uniform(0,population_size-1);
		}
		
		r2= random->uniform(0,population_size-1);

		while(r2== individual)
		{
			r2= random->uniform(0,population_size-1);
		}
		
		r3= random->uniform(0,population_size-1);

		while(r3== individual)
		{
			r3= random->uniform(0,population_size-1);
		}

		for(i=0;i<problem->problem_size;++i)
		{
			F=random->uniform(0.0,1.0);
			trial_vector[i]= population[r1][i] + F*(population[r2][i] - population[r3][i]);
		}
	}
	else
	{
		int r1, r2, r3; //index of individuals

		r1= random->uniform(0,population_size-1);
		r2= random->uniform(0,population_size-1);
		r3= random->uniform(0,population_size-1);

		for(i=0;i<problem->problem_size;++i)
		{
			//F=random->uniform(0.0,1.0);
			trial_vector[i]= population[r1][i] + F*(population[r2][i] - population[r3][i]);
		
		}
	}
}

void Differential_Evolution::crossoverOperator(int individual)
{
	int i;
	
	//choose a random index, to ensure that at least this will be inserted in the trial_vector
	int random_index= random->uniform(0,problem->problem_size-1);
	
	for(i=0;i<problem->problem_size;++i)
	{

		if(random_index != i && random->uniform() > CR)
		{
			trial_vector[i]= population[individual][i];
		}
		else
		{
		//	trial_vector[i]= mutation_vector[i];
//			if(trial_vector[i]>max_limit || trial_vector[i]<min_limit)
//			{
//				trial_vector[i]= generateRandomVariable(i);
//			}
		}
	}
}

void Differential_Evolution::printParameters()
{
	printf("population size %d\n",population_size);
	printf("max_limit %f\n",max_limit);
	printf("min_limit %f\n",min_limit);
	printf("F %f\n", F);
	printf("CR %f\n", CR);
}

double Differential_Evolution::optimize(Optimization_Problem* optimization_problem, int number_of_problems, double** solution)
{
	int i;
		
	problem= optimization_problem;

	if(number_of_problems!=1)
	{
		printf("ERROR: Differential Evolution can only solve one problem at a time, it is not multi-objective\n");
		exit(1);
	}
	
	/////////////// Allocating memory////////////////////

	//if no problem was defined before, just allocate the memory
	//otherwise, check if there is already available from last problem and manage memory accordingly
	if(problem_previously_defined==false)
		
	{
		allocateProblemMemory();
	}
	else
	{
		//if the allocated memory is not enough
		if(allocated_problem_size < problem->problem_size)
		{
			if(keep_previous_individuals == false)
			{
				freeProblemMemory();

				allocateProblemMemory();
			}
			else
			{
				reallocateProblemMemory();
			}
		}
	}


	///////////////// initialization generate random population //////////////////////
	for(i=0;i<population_size;++i)
	{	
		//if it is the first time it is evolving or any occurence that does not defined the problem before 
		//or if it was not set to keep previous individuals
		//then generate full random individuals
		if(problem_previously_defined==false || keep_previous_individuals == false)
		{
			generateRandomIndividual(i);
		}
		//otherwise, generate partially random individuals
		else
		{
			generatePartiallyRandomIndividual(i);
		}
	}
	
	//from this point on, it is considered that the problem was previously defined 
	//since anything past this point should work regardless
	problem_previously_defined=true;	

	///////////////////Generation's loop////////////////////////////////////////

	for(i=0;i<max_generations;i++)
	{
		newGeneration();
	}
	
	int best_individual= getBestIndividual();
	double best_fitness;
	problem->objectiveFunction(population[best_individual], &best_fitness);

	//copy solution to the vector of solution passed
	memcpy(*solution,population[best_individual],problem->problem_size*sizeof(double));
	
	
	//update the size of the previous problem 
	last_problem_size=problem->problem_size;


	return best_fitness;

}

double Differential_Evolution::optimizeDebug(Optimization_Problem* optimization_problem, int number_of_problems, double** solution)
{
	int i;
	
	//printParameters();	

	if(number_of_problems!=1)
	{
		printf("ERROR: Differential Evolution can only solve one problem at a time, it is not multi-objective\n");
		exit(1);
	}

	
	/////////////// Allocating memory////////////////////

	problem= optimization_problem;
	
	//if no problem was defined before, just allocate the memory
	//otherwise, check if there is already available from last problem and manage memory accordingly
	if(problem_previously_defined==false)
	{
		allocateProblemMemory();
	}
	else
	{
		//if the allocated memory is not enough
		if(allocated_problem_size < problem->problem_size)
		{
			if(keep_previous_individuals == false)
			{
				freeProblemMemory();

				allocateProblemMemory();
			}
			else
			{
				printf("reallocating\n");

				reallocateProblemMemory();
			}
		}
	}
	
	///////////////// initialization generate random population //////////////////////
	

	for(i=0;i<population_size;++i)
	{	
		//if it is the first time it is evolving or any occurence that does not defined the problem before 
		//or if it was not set to keep previous individuals
		//then generate full random individuals
		if(problem_previously_defined==false || keep_previous_individuals == false)
		{
			generateRandomIndividual(i);
		}
		else
		{
			generatePartiallyRandomIndividual(i);
		}
	
		//printf("random %.2f\n",random.uniform(0.0,0.99));
		
		problem->objectiveFunction(population[i], &fitness[i]);
	}

	plotHistogram("fitness", fitness,population_size);
	
	int best_individual= getBestIndividual();
	double best_fitness;
	problem->objectiveFunction(population[best_individual], &best_fitness);

	printf("INITIAL best fitness found: %f\n",best_fitness);
	double discard_result;
	problem->printDetails(population[best_individual], &discard_result);
	
	//from this point on, it is considered that the problem was previously defined 
	//since anything past this point should work regardless
	problem_previously_defined=true;	

	///////////////////BEGIN LOOP////////////////////////////////////////

	for(i=0;i<max_generations;i++)
	{
//		char filename[64];

		for(int j=0;j<population_size;++j)
		{	
			problem->objectiveFunction(population[j], &fitness[j]);
		}

		//sprintf(filename,"fitness_before_gen_%d",i);

		newGeneration();
		
//		sprintf(filename,"fitness_gen_%d",i);
//		plotHistogram(filename, fitness,population_size);
		best_individual= getBestIndividual();
		problem->objectiveFunction(population[best_individual], &best_fitness);

		printf("generation %d best fitness %f\n", i, best_fitness);
	
	}
	
//	for(i=0;i<population_size;++i)
//	{	
//		fitness[i]= problem->objectiveFunction(population[i]);
//	}
//	plotHistogram("fitnessCLuster", fitness,population_size);
	
	best_individual= getBestIndividual();
	problem->objectiveFunction(population[best_individual], &best_fitness);

	printf("best fitness found: %f\n",best_fitness);
	problem->printDetails(population[best_individual], &discard_result);

	//copy solution to the vector of solution passed
	memcpy(*solution,population[best_individual],problem->problem_size*sizeof(double));
//	printArray(population[best_individual], problem->problem_size);
//	printArray(*solution, problem->problem_size);
	

	//freeing memory
	//free(trial_vector);

	//update the size of the previous problem 
	last_problem_size=problem->problem_size;

	return best_fitness;

}

int Differential_Evolution::newGeneration()
{
	int i;


	for(i=0;i<population_size;++i)
	{
		//create the mutant vector
		mutationOperator(i);	

		//create the trial vector, based on the mutant vector and original vector
		crossoverOperator(i);
		
		double fitness_trial, fitness_father;
		problem->objectiveFunction(trial_vector, &fitness_trial);
		problem->objectiveFunction(population[i], &fitness_father);

		//selection, test if the trial vector is better than the original vector
		if(fitness_trial > fitness_father)
		{
			//copy the trial to the original vector
			memcpy(population[i], trial_vector, problem->problem_size*sizeof(double));
		}
	}

	generation++;

	return 0;
}

int Differential_Evolution::getBestIndividual()
{
	int i;
	int best_index=0;
	double best_fitness;
	problem->objectiveFunction(population[best_index], &best_fitness);

	for(i=1;i<population_size;++i)
	{	
		double individual_fitness;
		problem->objectiveFunction(population[i], &individual_fitness);

		if(individual_fitness > best_fitness)
		{
			best_index= i;
			problem->objectiveFunction(population[best_index], &best_fitness);
		}
	}
	
	return best_index;
}
	
void Differential_Evolution::generateRandomIndividual(int individual)
{
	int j;

	for(j=0;j<problem->problem_size;++j)
	{
		population[individual][j]= generateRandomVariable(j);
	}
}

//only initialize with random where it was not previously evolved
void Differential_Evolution::generatePartiallyRandomIndividual(int individual)
{
	int j;

	for(j=last_problem_size; j<problem->problem_size; ++j)
	{
		population[individual][j]= generateRandomVariable(j);
	}
	
}

double Differential_Evolution::generateRandomVariable(int variable)
{
	return (random->uniform(0.0,1.0)*(max_limit - min_limit) + min_limit);	
}

void Differential_Evolution::allocatePopulations()
{
	int i;

	for(i=0;i<population_size;++i)
	{	
		population[i]=(double*)malloc(sizeof(double)*allocated_problem_size);
	}
		
}

//Used when one wants to keep the individuals from last generation
void Differential_Evolution::reallocateProblemMemory()
{
	int i;

	allocated_problem_size= problem->problem_size*2;
	
	trial_vector=(double*)realloc(trial_vector,sizeof(double)*allocated_problem_size);
	
	//reallocating population
	for(i=0;i<population_size;++i)
	{	
		population[i]=(double*)realloc(population[i],sizeof(double)*allocated_problem_size);
	}
	
}

//for each new type of problem, this function will allocate memory
void Differential_Evolution::allocateProblemMemory()
{
	allocated_problem_size= problem->problem_size*2;
	
	trial_vector=(double*)malloc(sizeof(double)*allocated_problem_size);
	
	//allocating 
	allocatePopulations();
	
}

//for each new type of problem, this function will allocate memory
void Differential_Evolution::freeProblemMemory()
{
	int i;

	free(trial_vector);

	for(i=0;i<population_size;++i)
	{	
		free(population[i]);
	}
	
}

// new_fitness > best_fitness ? And treat nans
/*bool Differential_Evolution::isGreater(double new_fitness, double best_fitness)
{
	//if best_fitness is nan
	//or if new_fitness is not nan and new_fitness is greater than best_fitness
	if( (isnan(best_fitness)) || (!isnan(new_fitness) && new_fitness > best_fitness) )
	{
		return true;
	}
	else
	{
		return false;
	}
}


// new_fitness < best_fitness ? And treat nans
bool Differential_Evolution::isLower(double new_fitness, double best_fitness)
{
	//if best_fitness is nan
	//or if new_fitness is not nan and new_fitness is greater than best_fitness
	if( (isnan(best_fitness)) || (!isnan(new_fitness) && new_fitness < best_fitness) )
	{
		return true;
	}
	else
	{
		return false;
	}
}

*/
