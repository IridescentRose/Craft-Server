#pragma once
#include <map>
#include <algorithm>

template<typename K, typename V>
std::map<V, K> inverse_map(std::map<K, V>& map)
{
	std::map<V, K> inv;
	std::for_each(map.begin(), map.end(),
		[&inv](const std::pair<K, V>& p)
		{
			inv.insert(std::make_pair(p.second, p.first));
		});
	return inv;
}
