-- backup
drop table if exists creature_template_portal_backup;
create table creature_template_portal_backup 
SELECT * FROM creature_template where SubName = 'Portal Trainer';

-- cleanup
DELETE FROM `gossip_menu_option` where option_text = 'Open portal for my party';

-- add to trainers
INSERT INTO `gossip_menu_option`
(`menu_id`, `id`, `option_icon`, `option_text`, `option_id`, `npc_option_npcflag`, `action_menu_id`, `action_poi_id`, `action_script_id`, `box_coded`, `box_money`, `box_text`, `condition_id`) 
SELECT menu_id, 71, 5, 'Open portal for my party', 18, 16, 0, 40, 0, 0, 0, '', 0 FROM `gossip_menu_option` where menu_id in (4822,4823,4827,4821,4825,4826);

-- add missing gossips to trainers
-- update creature_template set GossipMenuId = 4537 where entry in (select entry from creature_template_portal_backup);

SELECT * FROM creature_template where SubName = 'Portal Trainer';