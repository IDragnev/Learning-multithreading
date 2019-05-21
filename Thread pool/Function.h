#ifndef __FUNCTION_WRAPPER_H_INCLUDED__
#define __FUNCTION_WRAPPER_H_INCLUDED__

#include <memory>

namespace IDragnev::Multithreading
{
	class Function
	{
	private:
		class Functor
		{
		public:
			virtual ~Functor() = default;
			virtual void invoke() = 0;
		};

		template <typename Callable>
		class SpecificFunctor : public Functor
		{
		private:
			template <typename F>
			using EnableIfMatchesCallable = 
				std::enable_if_t<std::is_same_v<std::decay_t<F>, Callable>>;

		public:
			template <typename F,
			          typename = EnableIfMatchesCallable<F>>
			SpecificFunctor(F&& f) : function(std::forward<F>(f)) { }

			void invoke() override { function(); }

		private:
			Callable function;
		};

	public:
		Function() = default;
		Function(Function&& source) = default;
		template <typename F>
		Function(F&& f) :
			functor(std::make_unique<SpecificFunctor<std::decay_t<F>>>(std::forward<F>(f)))
		{
		}

		Function& operator=(Function&& rhs) = default;

		void operator()() { functor->invoke(); }

	private:
		std::unique_ptr<Functor> functor;
	};
}

#endif //__FUNCTION_WRAPPER_H_INCLUDED__
