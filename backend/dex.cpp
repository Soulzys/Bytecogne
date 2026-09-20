#include "dex.h"
#include "cogne.h"
#include "utils.h"

void dex_paid_order_popup(PaidOrder::Properties& properties)
{
	if (ImGui::BeginPopupModal("Dex - Paid Order", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::Text("Choose the data you want to retrieve");
		ImGui::Separator();

		ImGui::CheckboxFlags("Type"             , &(unsigned int)properties, (unsigned int)PaidOrder::Properties::TYPE);
		ImGui::CheckboxFlags("Status"           , &(unsigned int)properties, (unsigned int)PaidOrder::Properties::STATUS);
		ImGui::CheckboxFlags("Payment timestamp", &(unsigned int)properties, (unsigned int)PaidOrder::Properties::PAYMENT_TIMESTAMP);

		if (ImGui::Button("Done"))
		{
			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}
}

std::string stringify(PaidOrder::Type data)
{
	std::string fields[4];
	fields[0] = "TOKEN_PROFILE";
	fields[1] = "COMMUNITY_TAKEOVER";
	fields[2] = "TOKEN_AD";
	fields[3] = "TRENDING_BAR_AD";

	return fields[(enum_type)data];
}

std::string stringify(PaidOrder::Status data)
{
	std::string fields[5];
	fields[0] = "PROCESSING";
	fields[1] = "CANCELLED";
	fields[2] = "ON_HOLD";
	fields[3] = "APPROVED";
	fields[4] = "REJECTED";

	return fields[(enum_type)data];
}

std::string stringify(const PaidOrder& data)
{
	using P = PaidOrder::Properties;

	std::string result = "PaidOrder\n{\n";


	std::string type_p              = "\ttype              : ";
	std::string status_p            = "\tstatus            : ";
	std::string payment_timestamp_p = "\tpayment timestamp : ";

	std::string type_str              = (has_flag<P>(data.internal_properties, P::TYPE))              ? 
		type_p              + stringify(data.type) + "\n"                                             : 
		type_p              + "unused\n";
	std::string status_str            = (has_flag<P>(data.internal_properties, P::STATUS))            ?
		status_p            + stringify(data.status) + "\n"                                           : 
		status_p            + "unused\n";
	std::string payment_timestamp_str = (has_flag<P>(data.internal_properties, P::PAYMENT_TIMESTAMP)) ?
		payment_timestamp_p + stringify(data.payment_timestamp) + "\n"                                : 
		payment_timestamp_p + "unused\n";

	result += type_str;
	result += status_str;
	result += payment_timestamp_str;

	result += "}\n";

	return result;
}

std::string stringify(int32 data)
{
	return std::to_string(data);
}

std::string stringify(uint32 flags, const std::string flags_name[FLAGS_COUNT], const std::string& prefix)
{
	std::string result = prefix + "{\n";

	for (int i = 0; i < 32; i++)
	{
		bool is_set = ((1 << i) & flags) != 0;
		if (is_set)
		{
			result += prefix + flags_name[i] + "\n";
		}
	}

	result += prefix + "}\n";

	return result;
}
