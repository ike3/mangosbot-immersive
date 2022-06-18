-- backup
drop table if exists creature_template_portal_backup;
create table creature_template_portal_backup 
SELECT * FROM creature_template where SubName = 'Portal Trainer';

-- cleanup
DELETE FROM `gossip_menu_option` where option_text = 'Open portal for my party';
DELETE FROM `gossip_menu_option` where option_text = 'Wait here for my party';

-- add to trainers
INSERT INTO `gossip_menu_option`
(`menu_id`, `id`, `option_icon`, `option_text`, `option_id`, `npc_option_npcflag`, `action_menu_id`, `action_poi_id`, `action_script_id`, `box_coded`, `box_money`, `box_text`, `condition_id`) 
SELECT menu_id, 71, 5, 'Open portal for my party', 18, 16, 0, 40, 0, 0, 0, '', 0 FROM `gossip_menu_option` where menu_id in (4822,4823,4827,4821,4825,4826);

INSERT INTO `gossip_menu_option`
(`menu_id`, `id`, `option_icon`, `option_text`, `option_id`, `npc_option_npcflag`, `action_menu_id`, `action_poi_id`, `action_script_id`, `box_coded`, `box_money`, `box_text`, `condition_id`) 
SELECT entry, 71, 5, 'Open portal for my party', 18, 16, 0, 40, 0, 0, 0, '', 0 FROM `gossip_menu` where entry in (4822,4823,4827,4821,4825,4826) and 
not exists (SELECT * FROM `gossip_menu_option` where menu_id = entry and id = 71);

INSERT INTO `gossip_menu_option`
(`menu_id`, `id`, `option_icon`, `option_text`, `option_id`, `npc_option_npcflag`, `action_menu_id`, `action_poi_id`, `action_script_id`, `box_coded`, `box_money`, `box_text`, `condition_id`) 
SELECT entry, 0, 3, 'Please teach me.', 5,	16,	0,	0,	0,	0,	0,	'', 0 FROM `gossip_menu` where entry in (4822,4823,4827,4821,4825,4826) and 
not exists (SELECT * FROM `gossip_menu_option` where menu_id = entry and id = 0);

SELECT * FROM creature_template where SubName = 'Portal Trainer';

-- add to zeppelin masters
INSERT INTO `gossip_menu_option`
(`menu_id`, `id`, `option_icon`, `option_text`, `option_id`, `npc_option_npcflag`, `action_menu_id`, `action_poi_id`, `action_script_id`, `box_coded`, `box_money`, `box_text`, `condition_id`) 
SELECT menu_id, 71, 5, 'Wait here for my party', 18, 1, 0, 41, 0, 0, 0, '', 0 FROM `gossip_menu_option` where menu_id in (1971,2101,1969,1971,3841,1969);

INSERT INTO `gossip_menu_option`
(`menu_id`, `id`, `option_icon`, `option_text`, `option_id`, `npc_option_npcflag`, `action_menu_id`, `action_poi_id`, `action_script_id`, `box_coded`, `box_money`, `box_text`, `condition_id`) 
SELECT entry, 71, 5, 'Wait here for my party', 18, 1, 0, 41, 0, 0, 0, '', 0 FROM `gossip_menu` where entry in (3149,3150,9564,9566,12136,12137) and 
not exists (SELECT * FROM `gossip_menu_option` where menu_id = entry and id = 71);

SELECT * FROM creature_template where SubName = 'Zeppelin Master'; 

-- Thamarian
SELECT * FROM creature_template where name = 'Thamarian'; 
INSERT INTO `gossip_menu_option`
(`menu_id`, `id`, `option_icon`, `option_text`, `option_id`, `npc_option_npcflag`, `action_menu_id`, `action_poi_id`, `action_script_id`, `box_coded`, `box_money`, `box_text`, `condition_id`) 
SELECT entry, 0, 3, 'Wait here for my party', 18, 3, 0, 41, 0, 0, 0, '', 0 FROM `gossip_menu` where entry in (4308) and 
not exists (SELECT * FROM `gossip_menu_option` where menu_id = entry and id = 0);

-- Grimble
SELECT * FROM creature_template where name = 'Grimble'; 
INSERT INTO `gossip_menu_option`
(`menu_id`, `id`, `option_icon`, `option_text`, `option_id`, `npc_option_npcflag`, `action_menu_id`, `action_poi_id`, `action_script_id`, `box_coded`, `box_money`, `box_text`, `condition_id`) 
SELECT entry, 0, 3, 'Wait here for my party', 18, 1, 0, 41, 0, 0, 0, '', 0 FROM `gossip_menu` where entry in (1967) and 
not exists (SELECT * FROM `gossip_menu_option` where menu_id = entry and id = 0);

-- Grizzlowe
SELECT * FROM creature_template where name = 'Grizzlowe'; 
INSERT INTO `gossip_menu_option`
(`menu_id`, `id`, `option_icon`, `option_text`, `option_id`, `npc_option_npcflag`, `action_menu_id`, `action_poi_id`, `action_script_id`, `box_coded`, `box_money`, `box_text`, `condition_id`) 
SELECT entry, 0, 3, 'Wait here for my party', 18, 1, 0, 41, 0, 0, 0, '', 0 FROM `gossip_menu` where entry in (1968) and 
not exists (SELECT * FROM `gossip_menu_option` where menu_id = entry and id = 0);

-- Adam Lind
SELECT * FROM creature_template where name = 'Adam Lind'; 
INSERT INTO `gossip_menu_option`
(`menu_id`, `id`, `option_icon`, `option_text`, `option_id`, `npc_option_npcflag`, `action_menu_id`, `action_poi_id`, `action_script_id`, `box_coded`, `box_money`, `box_text`, `condition_id`) 
SELECT entry, 0, 3, 'Wait here for my party', 18, 1, 0, 41, 0, 0, 0, '', 0 FROM `gossip_menu` where entry in (4343) and 
not exists (SELECT * FROM `gossip_menu_option` where menu_id = entry and id = 0);

SELECT * FROM `gossip_menu` where entry in (4308);
SELECT * FROM `gossip_menu_option` where menu_id in (4308);