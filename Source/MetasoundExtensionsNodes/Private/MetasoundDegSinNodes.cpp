// Copyright Hitbox Games, LLC. All Rights Reserved.

#include "MetasoundAudioBuffer.h"
#include "MetasoundExecutableOperator.h"
#include "MetasoundFacade.h"
#include "MetasoundNode.h"
#include "MetasoundNodeRegistrationMacro.h"
#include "MetasoundParamHelper.h"
#include "MetasoundPrimitives.h"
#include "MetasoundStandardNodesCategories.h"
#include "MetasoundStandardNodesNames.h"
#include "MetasoundTime.h"

#define LOCTEXT_NAMESPACE "MetasoundStandardNodes_DegSinNode"

namespace Metasound
{
	namespace DegSinVertexNames
	{
		METASOUND_PARAM(InputValue, "Input", "Input value.");
		METASOUND_PARAM(OutputValue, "Value", "The solved sine of the input.");
	}

	namespace MetasoundDegSinNodePrivate
	{
		FNodeClassMetadata CreateNodeClassMetadata(const FName& InDataTypeName, const FName& InOperatorName, const FText& InDisplayName, const FText& InDescription, const FVertexInterface& InDefaultInterface)
		{
			FNodeClassMetadata Metadata
			{
				FNodeClassName{"DegSin", InOperatorName, InDataTypeName},
				1, // Major Version
				0, // Minor Version
				InDisplayName,
				InDescription,
				PluginAuthor,
				PluginNodeMissingPrompt,
				InDefaultInterface,
				{NodeCategories::Math},
				{},
				FNodeDisplayStyle{}
			};

			return Metadata;
		}

		template <typename ValueType>
		struct TDegSin
		{
			bool bSupported = false;
		};

		template <>
		struct TDegSin<int32>
		{
			static void GetDegSin(const int32 In, int32& OutDegSin)
			{
				OutDegSin = FMath::Sin(UE_DOUBLE_PI / (180.0) * static_cast<float>(In));
			}

			static TDataReadReference<int32> CreateInRef(const FBuildOperatorParams& InParams)
			{
				using namespace DegSinVertexNames;
				const FInputVertexInterfaceData& InputData = InParams.InputData;
				return InputData.GetOrCreateDefaultDataReadReference<int32>(METASOUND_GET_PARAM_NAME(InputValue), InParams.OperatorSettings);
			}
		};

		template <>
		struct TDegSin<float>
		{
			static void GetDegSin(const float In, float& OutDegSin)
			{
				OutDegSin = FMath::Sin(UE_DOUBLE_PI / (180.0) * In);
			}

			static TDataReadReference<float> CreateInRef(const FBuildOperatorParams& InParams)
			{
				using namespace DegSinVertexNames;
				const FInputVertexInterfaceData& InputData = InParams.InputData;
				return InputData.GetOrCreateDefaultDataReadReference<float>(METASOUND_GET_PARAM_NAME(InputValue), InParams.OperatorSettings);
			}

			static bool IsAudioBuffer() { return false; }
		};

		template <>
		struct TDegSin<FTime>
		{
			static void GetDegSin(const FTime& In, FTime& OutDegSin)
			{
				OutDegSin = FTime(FMath::Sin(UE_DOUBLE_PI / (180.0) * In.GetSeconds()));
			}

			static TDataReadReference<FTime> CreateInRef(const FBuildOperatorParams& InParams)
			{
				using namespace DegSinVertexNames;
				const FInputVertexInterfaceData& InputData = InParams.InputData;
				return InputData.GetOrCreateDefaultDataReadReference<FTime>(METASOUND_GET_PARAM_NAME(InputValue), InParams.OperatorSettings);
			}

			static bool IsAudioBuffer() { return false; }
		};

		// 		template<>
		// 		struct TDegSin<FAudioBuffer>
		// 		{
		// 			static void GetDegSin(const FAudioBuffer& In, FAudioBuffer& OutDegSin)
		// 			{
		// 				TArrayView<float> OutDegSinView(OutDegSin.GetData(), OutDegSin.Num());
		// 				TArrayView<const float> InView(In.GetData(), OutDegSin.Num());
		// 				
		// 				Audio::ArrayDegSin(InView, OutDegSinView);
		// 			}
		//
		// 			static TDataReadReference<FAudioBuffer> CreateInRef(const FBuildOperatorParams& InParams)
		// 			{
		// 				using namespace DegSinVertexNames;
		// 				const FInputVertexInterfaceData& InputData = InParams.InputData;
		// 				return InputData.GetOrCreateDefaultDataReadReference<FAudioBuffer>(METASOUND_GET_PARAM_NAME(InputValue), InParams.OperatorSettings);
		// 			}
		// 		};
	}

	template <typename ValueType>
	class TDegSinNodeOperator : public TExecutableOperator<TDegSinNodeOperator<ValueType>>
	{
	public:
		static const FVertexInterface& GetDefaultInterface()
		{
			using namespace DegSinVertexNames;
			using namespace MetasoundDegSinNodePrivate;

			static const FVertexInterface DefaultInterface(
				FInputVertexInterface(
					TInputDataVertex<ValueType>(METASOUND_GET_PARAM_NAME_AND_METADATA(InputValue))
				),
				FOutputVertexInterface(
					TOutputDataVertex<ValueType>(METASOUND_GET_PARAM_NAME_AND_METADATA(OutputValue))
				)
			);

			return DefaultInterface;
		}

		static const FNodeClassMetadata& GetNodeInfo()
		{
			auto CreateNodeClassMetadata = []() -> FNodeClassMetadata
			{
				const FName DataTypeName = GetMetasoundDataTypeName<ValueType>();
				const FName OperatorName = TEXT("DegSin");
				const FText NodeDisplayName = METASOUND_LOCTEXT_FORMAT("DegSinDisplayNamePattern", "Sin (degrees) ({0})", GetMetasoundDataTypeDisplayText<ValueType>());
				const FText NodeDescription = METASOUND_LOCTEXT("DegSinDesc", "Returns the sine of the input value.");
				const FVertexInterface NodeInterface = GetDefaultInterface();

				return MetasoundDegSinNodePrivate::CreateNodeClassMetadata(DataTypeName, OperatorName, NodeDisplayName, NodeDescription, NodeInterface);
			};

			static const FNodeClassMetadata Metadata = CreateNodeClassMetadata();
			return Metadata;
		}

		static TUniquePtr<IOperator> CreateOperator(const FBuildOperatorParams& InParams, FBuildResults& OutResults)
		{
			using namespace DegSinVertexNames;
			using namespace MetasoundDegSinNodePrivate;

			TDataReadReference<ValueType> Input = TDegSin<ValueType>::CreateInRef(InParams);

			return MakeUnique<TDegSinNodeOperator<ValueType>>(InParams.OperatorSettings, Input);
		}


		TDegSinNodeOperator(const FOperatorSettings& InSettings,
		                    const TDataReadReference<ValueType>& InInput)
			: Input(InInput)
			  , OutputValue(TDataWriteReferenceFactory<ValueType>::CreateAny(InSettings))
		{
			GetDegSin();
		}

		virtual ~TDegSinNodeOperator() = default;


		virtual void BindInputs(FInputVertexInterfaceData& InOutVertexData) override
		{
			using namespace DegSinVertexNames;
			InOutVertexData.BindReadVertex(METASOUND_GET_PARAM_NAME(InputValue), Input);
		}

		virtual void BindOutputs(FOutputVertexInterfaceData& InOutVertexData) override
		{
			using namespace DegSinVertexNames;
			InOutVertexData.BindReadVertex(METASOUND_GET_PARAM_NAME(OutputValue), OutputValue);
		}

		void GetDegSin()
		{
			using namespace MetasoundDegSinNodePrivate;

			TDegSin<ValueType>::GetDegSin(*Input, *OutputValue);
		}

		void Execute()
		{
			GetDegSin();
		}

		void Reset(const IOperator::FResetParams& InParams)
		{
			GetDegSin();
		}

	private:
		TDataReadReference<ValueType> Input;
		TDataWriteReference<ValueType> OutputValue;
	};

	/** TDegSinNode
	 *
	 *  Returns the sine of the input value.
	 */
	template <typename ValueType>
	using TDegSinNode = TNodeFacade<TDegSinNodeOperator<ValueType>>;

	using FDegSinNodeInt32 = TDegSinNode<int32>;
	METASOUND_REGISTER_NODE(FDegSinNodeInt32)

	using FDegSinNodeFloat = TDegSinNode<float>;
	METASOUND_REGISTER_NODE(FDegSinNodeFloat)

	using FDegSinNodeTime = TDegSinNode<FTime>;
	METASOUND_REGISTER_NODE(FDegSinNodeTime)

	// using FDegSinNodeAudioBuffer = TDegSinNode<FAudioBuffer>;
	// METASOUND_REGISTER_NODE(FDegSinNodeAudioBuffer)
}

#undef LOCTEXT_NAMESPACE
